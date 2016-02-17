#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <queue>
#include <functional>   // std::bind
#include <cassert>

#include "knapsack.h"

// turn on this if you want to see the detail
//#define FengLeman_DEBUG

// turn on this if you want to get the output of drones
#define WRITE_OUTPUT

//#define DRAW_IMG

#ifndef srcPath
#define srcPath "."
#endif

using namespace std;

string inputFile = srcPath "/busy_day.in";
//string inputFile = srcPath "/mother_of_all_warehouses.in";
//string inputFile = srcPath "/redundancy.in";

string solutionFile = inputFile+".solution";
ofstream output;

//parameter:
const int NearestClientsSearch=50;
const double droneWareDistanceMultiplier=2.8;
const double maxNeighborSearchingMultiplier=1.2;
//best paramter:
//busy: 50 2.8 1.2
//mother: 30 0 0.28
//redundancy: 20 1.7 1.2
typedef int TypeId;
typedef int CmdId;
typedef int WareId;
typedef int DroneId;
typedef int Weight;

int N, M;
int DroneCount;
int TotalTime;
Weight MaxLoad;
int TypeCount;
int WareCount;
int CmdCount;
int Score=0;
int currentDay;
vector<Weight> TypeWeight;
map<CmdId, vector<pair<WareId, TypeId> > >cmd_mapTo_FlowList;

class DistanceTable{
  vector<int> table;
  int maxSize;
  int distance(int x,int y){
    double d=sqrt(double(x)*double(x)+double(y)*double(y));
    d-=1e-5; // what if sqrt(4) = 2.0000001
    return int(ceil(d));
  }

public:
  DistanceTable(int maxsize):maxSize(maxsize){
    table.resize(maxSize*maxSize,-1);
    for(int i=0;i<maxSize;i++){
      for(int j=i;j<maxSize;j++){//i<=j
        table[i*maxSize+j]=distance(i,j);
      }
    }
  }
  int getDistance(int x1,int y1,int x2,int y2){
    int x=abs(x1-x2);
    int y=abs(y1-y2);
    if(x>y)swap(x,y);
    assert(x<maxSize && y<maxSize);
    return table[x*maxSize+y];
  }
};

int distanceRound(int x1,int y1,int x2,int y2){
  // store a table to accelerate the calculation of distance
  // (sqrt is time consuming)
  static DistanceTable table(max(N,M));
  return table.getDistance(x1,y1,x2,y2);
}

void finishOneCommand(){// and we add to the total score
  double ds=ceil((double(TotalTime - currentDay)) / TotalTime * 100);
  Score += ds;
  //cout<<"Score+="<<ds<<" ="<<Score<<endl;
}

struct WareHouse{
  WareId id;
  int x,y;
  vector<int> productAvailability;
  vector<int> productWaitingToBeLoaded;
  WareHouse():productWaitingToBeLoaded(TypeCount,0){

  }
  void prepareProductToBeLoaded(TypeId typeId){
    assert(productAvailability[typeId]>=1);
    productAvailability[typeId]--;
    productWaitingToBeLoaded[typeId]++;
  }

  void productLoadedOnDrone(TypeId typeId){
    assert(productWaitingToBeLoaded[typeId]>=1);
    productWaitingToBeLoaded[typeId]--;
  }
  void addProduct(TypeId typeId){
    productAvailability[typeId]++;
  }
};
vector<WareHouse> wareHouses;

struct Command{
  CmdId id;
  int x,y;
  multiset<TypeId> demands;
  multiset<TypeId> productsWaitingToReceive;
  int originTotalWeight;
  int originDemandCount;
  int demandsTotalWeight(){
    int sum=0;
    for(auto d : demands){
      sum+=TypeWeight[d];
    }
    return sum;
  }

  void productWillBeReceived(TypeId id){
    auto it = demands.find(id);
    assert(it!=demands.end());
    demands.erase(it);
    productsWaitingToReceive.insert(id);
  }
  void productReceived(TypeId typeId){
    auto it = productsWaitingToReceive.find(typeId);
    assert(it!=productsWaitingToReceive.end());
    productsWaitingToReceive.erase(it);
    if(demands.empty() && productsWaitingToReceive.empty()){
      finishOneCommand();
    }
  }
};
vector<Command> commands;

struct DroneOrder{
  enum DroneOrderType{
    Load, Unload, Deliver
  } orderType;
  WareId wareId;
  CmdId commandId;
  multiset<pair<TypeId,CmdId> > itemsToPickOrDrop;
};

struct DroneTask{
  enum DroneTaskType{
    Loading,
    Unloading,
    DropingToClient,
    WaytoWareHouse,
    WaytoClient,
  } taskType;
  int timeToFinish;
  WareId wareId;
  CmdId commandId;
  multiset<pair<TypeId,CmdId> > itemsToPickOrDrop;
};

struct Drone{
  DroneId id;
  int x,y;
  double xSimu,ySimu;
  double dx,dy;
  queue<DroneOrder> orderQueue;
  DroneTask currentTask;
  multiset<pair<TypeId,CmdId> > loadedProducts;
  bool isDoingATask;
  bool isBusy;
  Drone():isDoingATask(false),isBusy(false){

  }

  Weight loadedWeight(){
    Weight sum=0;
    for(auto id : loadedProducts){
      sum+=TypeWeight[id.first];
    }
    return sum;
  }
  void setXY(int x,int y){
    this->x=x;
    this->y=y;
#ifdef DRAW_IMG
    this->xSimu=x;
    this->ySimu=y;
#endif
  }
  void setDirection(int x0,int y0){
    dx=x0-x;
    dy=y0-y;
    double l=sqrt(dx*dx+dy*dy);
    dx/=l;
    dy/=l;
  }

  void eraseProduct(pair<TypeId,CmdId> id){
    auto it = loadedProducts.find(id);
    assert(it!=loadedProducts.end());
    loadedProducts.erase(it);
  }
  void addProduct(pair<TypeId,CmdId> id){
    loadedProducts.insert(id);
    assert(this->loadedWeight()<=MaxLoad);
  }
  void makeOneTurn(){
    this->isBusy=false;
    if(!this->isDoingATask){
#ifdef FengLeman_DEBUG
      //cout<<"drone "<<this->id<<" need new task."<<endl;
#endif
      this->takeNextOrder_makeItATask();// it may become "Busy" after this call
    }
    if(this->isDoingATask){
      this->isBusy=true;
      this->doTask();
    }
  }

  void giveOneOrder(DroneOrder order){
    orderQueue.push(order);
#ifdef WRITE_OUTPUT
    switch(order.orderType){
    case DroneOrder::Deliver:
      for(auto item : order.itemsToPickOrDrop)
        output<<id<<" D "<<order.commandId<<" "<<item.first<<" 1"<<endl;
      break;
    case DroneOrder::Load:
      for(auto item : order.itemsToPickOrDrop)
        output<<id<<" L "<<order.wareId<<" "<<item.first<<" 1"<<endl;
      break;
    case DroneOrder::Unload:
      for(auto item : order.itemsToPickOrDrop)
        output<<id<<" U "<<order.wareId<<" "<<item.first<<" 1"<<endl;
      break;
    }
#endif
#ifdef FengLeman_DEBUG
    switch(order.orderType){
    case DroneOrder::Deliver:
      cout<<"drone "<<id<<" get order : Deliver to Client"<<order.commandId<<endl;
      break;
    case DroneOrder::Load:
      cout<<"drone "<<id<<" get order : Load at Ware"<<order.wareId<<endl;
      for(auto item : order.itemsToPickOrDrop){
        cout<<"Item"<<item.first<<"->Client"<<item.second<<' ';
      }
      cout<<endl;
      break;
    case DroneOrder::Unload:
      cout<<"drone "<<id<<" get order : Unload at Ware"<<order.wareId<<endl;
      break;
    }
#endif

  }
  void arrivedNextTaskPosition(){
    DroneOrder &nextOrder = orderQueue.front();
    switch(nextOrder.orderType){
    case DroneOrder::Load :
    case DroneOrder::Unload :{
      this->setXY(wareHouses[nextOrder.wareId].x,wareHouses[nextOrder.wareId].y);
      break;
    }
    case DroneOrder::Deliver:{
      this->setXY(commands[nextOrder.commandId].x,commands[nextOrder.commandId].y);
      break;
    }
    }
  }
  void setTask_GotoWareHouse(WareId id){
    currentTask.taskType=DroneTask::WaytoWareHouse;
    currentTask.wareId=id;
    WareHouse &w = wareHouses[id];
    currentTask.timeToFinish=distanceRound(x,y,w.x,w.y);
    currentTask.itemsToPickOrDrop.clear();
    currentTask.commandId=-1;
#ifdef DRAW_IMG
    setDirection(w.x,w.y);
#endif
  }
  void setTask_GotoClient(CmdId id){
    currentTask.taskType=DroneTask::WaytoClient;
    currentTask.commandId=id;
    Command &c = commands[id];
    currentTask.timeToFinish=distanceRound(x,y,c.x,c.y);
    currentTask.itemsToPickOrDrop.clear();
    currentTask.wareId=-1;
#ifdef DRAW_IMG
    setDirection(c.x,c.y);
#endif
  }

  void takeNextOrder_makeItATask(){
    if(orderQueue.empty()){
      this->isDoingATask=false;
      return;
    }else{
      this->isDoingATask=true;
    }
    DroneOrder order = orderQueue.front();

    switch(order.orderType){
    case DroneOrder::Load:
    {
      if(x==wareHouses[order.wareId].x && y==wareHouses[order.wareId].y){
        currentTask.taskType=DroneTask::Loading;
        currentTask.wareId=order.wareId;
        currentTask.commandId=order.commandId;
        currentTask.timeToFinish=1;
        currentTask.itemsToPickOrDrop=order.itemsToPickOrDrop;
        orderQueue.pop();
#ifdef FengLeman_DEBUG
        cout<<"drone "<<this->id<<", new task : Load at Ware"
           <<order.wareId<<endl;
#endif

      }else{
        setTask_GotoWareHouse(order.wareId);
#ifdef FengLeman_DEBUG
        cout<<"drone "<<this->id<<", new task : FlyTo Ware"
           <<order.wareId<<endl;
#endif
      }
      break;
    }
    case DroneOrder::Unload:
    {
      if(x==wareHouses[order.wareId].x && y==wareHouses[order.wareId].y){
        currentTask.taskType=DroneTask::Unloading;
        currentTask.wareId=order.wareId;
        currentTask.timeToFinish=1;
        currentTask.itemsToPickOrDrop=order.itemsToPickOrDrop;
        orderQueue.pop();
#ifdef FengLeman_DEBUG
        cout<<"drone "<<this->id<<", new task : Unload at Ware"
           <<order.wareId<<endl;
#endif
      }else{
        setTask_GotoWareHouse(order.wareId);
#ifdef FengLeman_DEBUG
        cout<<"drone "<<this->id<<", new task : FlyTo Ware"
           <<order.wareId<<endl;
#endif
      }
      break;
    }
    case DroneOrder::Deliver:
    {
      if(x==commands[order.commandId].x && y==commands[order.commandId].y){
        currentTask.taskType=DroneTask::DropingToClient;
        currentTask.commandId=order.commandId;
        currentTask.timeToFinish=1;
        currentTask.itemsToPickOrDrop=order.itemsToPickOrDrop;
        orderQueue.pop();
#ifdef FengLeman_DEBUG
        cout<<"drone "<<this->id<<", new task : Deliver at Client"
           <<order.commandId<<endl;
#endif
      }else{
        setTask_GotoClient(order.commandId);
#ifdef FengLeman_DEBUG
        cout<<"drone "<<this->id<<", new task : FlyTo Client"
           <<order.commandId<<endl;
#endif
      }
      break;
    }
    }
  }
  void doTask(){
    switch (currentTask.taskType){
    case DroneTask::Loading :{
      assert(x==wareHouses[currentTask.wareId].x && y==wareHouses[currentTask.wareId].y);
      for(auto id : currentTask.itemsToPickOrDrop){
        wareHouses[currentTask.wareId].productLoadedOnDrone(id.first);
        this->addProduct(id);
      }
      this->isDoingATask=false;
      break;
    }
    case DroneTask::Unloading :{
      assert(x==wareHouses[currentTask.wareId].x && y==wareHouses[currentTask.wareId].y);
      loadedProducts=currentTask.itemsToPickOrDrop;
      for(auto id : currentTask.itemsToPickOrDrop){
        this->eraseProduct(id);
        wareHouses[currentTask.wareId].addProduct(id.first);
      }
      this->isDoingATask=false;
      break;
    }
    case DroneTask::DropingToClient:{
      assert(x==commands[currentTask.commandId].x && y==commands[currentTask.commandId].y);
      loadedProducts=currentTask.itemsToPickOrDrop;
      for(auto id : currentTask.itemsToPickOrDrop){
        assert(id.second == currentTask.commandId);
        this->eraseProduct(id);
        commands[currentTask.commandId].productReceived(id.first);
      }
      this->isDoingATask=false;
      break;
    }
    case DroneTask::WaytoClient:
    case DroneTask::WaytoWareHouse:{
#ifdef DRAW_IMG
      xSimu+=dx;ySimu+=dy;
#endif
      currentTask.timeToFinish--;
      if(currentTask.timeToFinish==0){
        this->isDoingATask=false;
        this->arrivedNextTaskPosition();
        break;
      }
    }
    }

  }
};
vector<Drone> drones;



bool compareByWeight(const TypeId& id1, const TypeId& id2){
  return TypeWeight[id1]>TypeWeight[id2];
}

struct Travel{
  WareId wareId;
  CmdId commandId;
  multiset<TypeId> productsToDeliver;
  double score;
  double cmdPercent;
  double commandFinishPercentageTotal;
  int distance;
  int totalWeight;
  double weightPercent;
  bool operator <(const Travel & other)const{
    return score<other.score;
  }
};




void readFile(string inputFile){
  ifstream input(inputFile);
  input >> N >> M >> DroneCount >> TotalTime >> MaxLoad;
  input >> TypeCount;
  for(TypeId i=0;i<TypeCount;i++){
    Weight weight;
    input>>weight;
    TypeWeight.push_back(weight);
  }
  input >> WareCount;
  for(WareId i=0;i<WareCount;i++){
    WareHouse wareHouse;
    wareHouse.id=i;
    input >> wareHouse.x >> wareHouse.y;
    for(TypeId j=0;j<TypeCount;j++){
      int numProductAvailable;
      input>>numProductAvailable;
      wareHouse.productAvailability.push_back(numProductAvailable);
    }
    wareHouses.push_back(wareHouse);
  }
  input>>CmdCount;
  for(CmdId i=0;i<CmdCount;i++){
    Command cmd;
    cmd.id=i;
    input >> cmd.x >> cmd.y;
    int demandAmout;
    input>>demandAmout;
    for(int k=0;k<demandAmout;k++){
      TypeId typeId;
      input>>typeId;
      cmd.demands.insert(typeId);
    }
    cmd.originDemandCount=demandAmout;
    cmd.originTotalWeight=cmd.demandsTotalWeight();
    commands.push_back(cmd);
  }
  input.close();
}

void initializeDrone(){
  for(DroneId i=0;i<DroneCount;i++){
    Drone drone;
    drone.id=i;
    drone.setXY(wareHouses[0].x,wareHouses[0].y);
    drones.push_back(drone);
  }
}


map<CmdId,vector<CmdId> > buildNearestCmdsMap(int NearestCmdCount){
  NearestCmdCount=min(CmdCount-1,NearestCmdCount);
  map<CmdId,vector<CmdId> > cmd_mapto_nearestCmds;
  vector<CmdId> cmdIDs;
  for(CmdId i=0;i<CmdCount;i++){
    cmdIDs.push_back(i);
  }
  for(CmdId cmdId=0;cmdId<CmdCount;cmdId++){
    partial_sort(cmdIDs.begin(),cmdIDs.begin()+NearestCmdCount+1,cmdIDs.end(),
                 [cmdId](const CmdId& id1,const CmdId& id2){
      int d1=distanceRound(commands[id1].x,commands[id1].y,commands[cmdId].x,commands[cmdId].y);
      int d2=distanceRound(commands[id2].x,commands[id2].y,commands[cmdId].x,commands[cmdId].y);
      return d1<d2;
    }
    );
    int count=0;
    for(CmdId id : cmdIDs){
      //ignore itself
      if(id==cmdId){
        continue;
      }
      if(count<NearestCmdCount){
        cmd_mapto_nearestCmds[cmdId].push_back(id);
        count++;
      }else{
        break;
      }
    }
  }
  return cmd_mapto_nearestCmds;
}


multiset<TypeId> first_fit(multiset<TypeId>& products){
  multiset<pair<TypeId,int> > items;
  for(auto i : products){
    items.insert(make_pair(i,TypeWeight[i]));
  }
  multiset<TypeId> sol=knapsack(items,MaxLoad);
  return sol;
}

void buildProductFlowGraph(map<CmdId, vector<pair<WareId, TypeId> > >&cmd_mapTo_FlowList)
{
  cmd_mapTo_FlowList.clear();
  vector<WareHouse> wareHousesCopy=wareHouses;
  vector<CmdId> sortedId;
  for(CmdId k=0;k<CmdCount;k++) sortedId.push_back(k);
  sort(sortedId.begin(),sortedId.end(),[](CmdId id1,CmdId id2){
    return commands[id1].demands.size()<commands[id2].demands.size();
  });
  for(CmdId k : sortedId){
    Command &cmd = commands[k];

    for(auto itemId : cmd.demands){
      int minDistance=10000000;
      WareId ChosenWareId=-1;
      for(WareId i=0;i<WareCount;i++){
        WareHouse &thisWareHouse = wareHousesCopy[i];
        if(thisWareHouse.productAvailability[itemId]==0)
          continue;
        else{
          int distance=distanceRound(cmd.x,cmd.y,thisWareHouse.x,thisWareHouse.y);
          if(distance<minDistance){
            minDistance=distance;
            ChosenWareId=i;
          }
        }
      }
      assert(ChosenWareId!=-1);
      assert(wareHousesCopy[ChosenWareId].productAvailability[itemId]>0);
      wareHousesCopy[ChosenWareId].productAvailability[itemId]--;
      cmd_mapTo_FlowList[k].push_back(make_pair(ChosenWareId,itemId));
    }
  }
}



void giveBestTravels(vector<Travel>& travels)
{
  travels.clear();
  for(CmdId cmdId=0;cmdId<CmdCount;cmdId++){
    //find where does each product come from
    set<WareId> ChosenWareHouses;
    map<WareId, multiset<TypeId> > ware_map_TypeSet;
    vector<pair<WareId, TypeId> > &flowList=cmd_mapTo_FlowList[cmdId];
    for(auto arrow : flowList){
      WareId wareId=arrow.first;
      TypeId typeId=arrow.second;
      ChosenWareHouses.insert(wareId);
      ware_map_TypeSet[wareId].insert(typeId);
    }
    // now, in ChosenWareHouses, we have all the wareHouses where products come from
    // from each wareHouse, we want to find a best combination of products
    // whose weight sum is less than MaxLoad
    // this is a Knapsack problem problem (Probleme du sac a dos)
    // here we use FIRST_FIT
    for(auto wareId : ChosenWareHouses){
      multiset<TypeId> &products =  ware_map_TypeSet[wareId];
      multiset<TypeId> loadProduct=first_fit(products);
      Travel travel;
      travel.commandId=cmdId;
      travel.wareId=wareId;
      travel.distance = distanceRound(commands[cmdId].x,commands[cmdId].y,
                                      wareHouses[wareId].x,wareHouses[wareId].y);
      // the commandFinishPercentage is number of loadedProduct divided by total number of product
      // of the demand of this client
      int demandCount=commands[cmdId].demands.size();
      travel.cmdPercent=double(loadProduct.size())/flowList.size();
      travel.commandFinishPercentageTotal=1-double(demandCount-loadProduct.size())/
          commands[cmdId].originDemandCount;

      // the score of this travel is commandFinishPercentage divide by travel distance
      travel.score=travel.cmdPercent/travel.distance;
      travel.totalWeight=0;
      for(auto item : loadProduct){
        travel.productsToDeliver.insert(item);
        travel.totalWeight+=TypeWeight[item];
      }
      travel.weightPercent=travel.totalWeight/(double)commands[cmdId].demandsTotalWeight();

      travels.push_back(travel);
    }
  }
}

void reserveProducts(TypeId id,WareId ware,CmdId cmd){
  // reserve product, means we tell the wareHouse and the client that
  // certain product will be taken(resp. be deliver). They will record this so that
  // the command will not be processed a second time
  wareHouses[ware].prepareProductToBeLoaded(id);
  commands[cmd].productWillBeReceived(id);
}

void reserveProductsForTravel(Travel& travel){
  for(auto id : travel.productsToDeliver){
    reserveProducts(id,travel.wareId,travel.commandId);
  }
}

void dronePickTravel(vector<Travel>& travels, Drone& drone, vector<pair<DroneId,Travel> >& drone_travel){
  //for(auto &drone : drones){

  const double lambda = droneWareDistanceMultiplier;
  sort(travels.begin(),travels.end(),[drone,lambda](const Travel& t1,const Travel& t2){
    WareHouse& w1=wareHouses[t1.wareId];
    WareHouse& w2=wareHouses[t2.wareId];
    int distanceGoto1 = distanceRound(drone.x,drone.y,w1.x,w1.y);
    int distanceGoto2 = distanceRound(drone.x,drone.y,w2.x,w2.y);
    double score1 = t1.cmdPercent/(t1.distance+distanceGoto1*lambda);
    double score2 = t2.cmdPercent/(t2.distance+distanceGoto2*lambda);
    //double score1 = t1.commandFinishPercentageTotal/(t1.distance+distanceGoto1*lambda);
    //double score2 = t2.commandFinishPercentageTotal/(t2.distance+distanceGoto2*lambda);
    return score1<score2;
  });
  if(travels.empty()){
    return;
  }
  Travel bestTravel = travels.back();
  travels.pop_back();
  drone_travel.push_back(make_pair(drone.id,bestTravel));
  //remember to reserve at wareHouse and at client
  reserveProductsForTravel(bestTravel);
}

void droneConfirmTravel(Drone &drone, Travel travel,map<CmdId,vector<CmdId> >& nearestClient){
  // create two DroneOrder, one to load, one the deliver
  // order for load
  DroneOrder orderLoad;
  orderLoad.orderType=DroneOrder::Load;
  orderLoad.wareId=travel.wareId;
  orderLoad.commandId=travel.commandId;
  Weight loaded=0;
  for(auto typeId : travel.productsToDeliver){
    orderLoad.itemsToPickOrDrop.insert(make_pair(typeId,travel.commandId));
    loaded += TypeWeight[typeId];
  }

  // order for deliver
  DroneOrder orderDeliver;
  orderDeliver.orderType=DroneOrder::Deliver;
  orderDeliver.commandId=travel.commandId;
  for(auto typeId : travel.productsToDeliver){
    orderDeliver.itemsToPickOrDrop.insert(make_pair(typeId,travel.commandId));
  }

  //try to find some other product to deliver together in this travel
  Command &mainCmd = commands[travel.commandId];
  WareHouse &ware = wareHouses[travel.wareId];
  vector<DroneOrder> deliverOrders;
  //int lastX=mainCmd.x, lastY = mainCmd.y;
  vector<CmdId> nextNearestClient=nearestClient[travel.commandId];
  reverse(nextNearestClient.begin(),nextNearestClient.end());//nearest client is at the end

  //iterate on client's neighbour
  while(!nextNearestClient.empty()){
    CmdId cmdId=nextNearestClient.back();
    nextNearestClient.pop_back();
    // if its neighbour is too far away, we don't want to deliver to him
    //if(distanceRound(lastX,lastY,commands[cmdId].x,commands[cmdId].y)>30){
    if(distanceRound(mainCmd.x,mainCmd.y,commands[cmdId].x,commands[cmdId].y)
       >travel.distance*maxNeighborSearchingMultiplier){//<- change this parameter
      break;
    }

    // may be sort by the distance to the last visited client
    //    sort(nextNearestClient.begin(),nextNearestClient.end(),[cmdId](CmdId id1,CmdId id2){
    //      int d1=distanceRound(commands[cmdId].x,commands[cmdId].y,commands[id1].x,commands[id1].y);
    //      int d2=distanceRound(commands[cmdId].x,commands[cmdId].y,commands[id2].x,commands[id2].y);
    //      return d1>d2;
    //    });

    // create the order for deliver to neighbours
    DroneOrder orderDeliverOther;
    orderDeliverOther.orderType=DroneOrder::Deliver;
    orderDeliverOther.commandId=cmdId;
    vector<TypeId> sortedDemands;
    copy(commands[cmdId].demands.begin(),commands[cmdId].demands.end(),back_inserter(sortedDemands));
    sort(sortedDemands.begin(),sortedDemands.end(),compareByWeight);

    // we use also first-fit here to load neighbours' products
    for(auto typeId : sortedDemands){
      if(ware.productAvailability[typeId]>0 && TypeWeight[typeId]+loaded <= MaxLoad){
        loaded+=TypeWeight[typeId];
        orderLoad.itemsToPickOrDrop.insert(make_pair(typeId,cmdId));
        orderDeliverOther.itemsToPickOrDrop.insert(make_pair(typeId,cmdId));
        //remember to reserve at wareHouse and at client
        reserveProducts(typeId, ware.id, cmdId);
      }
    }
    if(orderDeliverOther.itemsToPickOrDrop.size()>0){
      deliverOrders.push_back(orderDeliverOther);
      //lastX=commands[cmdId].x;
      //lastY=commands[cmdId].y;
    }
  }


  drone.giveOneOrder(orderLoad);
  drone.giveOneOrder(orderDeliver);
  for(auto &order : deliverOrders){
    drone.giveOneOrder(order);
  }
}
void write(vector<unsigned char> & R,vector<unsigned char> & G,vector<unsigned char> & B
           ,int N,int M,int number);
void drawImg(int currentDay){
  vector<unsigned char> R(N*M,255);
  vector<unsigned char> G(N*M,255);
  vector<unsigned char> B(N*M,255);
  for(auto &w : wareHouses){
    R[w.x*M+w.y]=200;
    G[w.x*M+w.y]=200;
    B[w.x*M+w.y]=50;
  }
  for(auto &c : commands){
    if(c.demands.size()>0){
      R[c.x*M+c.y]=0;
      G[c.x*M+c.y]=0;
      B[c.x*M+c.y]=0;
    }
  }
  for(auto &d : drones){
    int x=round(d.xSimu);
    int y=round(d.ySimu);
    x=min(N,max(x,0));
    y=min(M,max(y,0));
    R[x*M+y]=0;
    G[x*M+y]=0;
    B[x*M+y]=255;
  }
  write(R,G,B,N,M,currentDay);
}

int main() {
  readFile(inputFile);
  initializeDrone();
#ifdef WRITE_OUTPUT
  output.open(solutionFile);
#endif

  //for each client, we want to know the 20 nearest clients to him
  map<CmdId,vector<CmdId> > nearestClient = buildNearestCmdsMap(NearestClientsSearch);


  vector<Travel> travels;

  //the timer, not important...
  clock_t time1, time2, timeStart,timeEnd;
  int last1000Day=0;
  time1=timeStart=clock();


  //The main loop of day(turn)
  //TotalTime=100000;
  bool noMoreDemand=false;
  bool weCanStop=false;
  for(currentDay=0;currentDay<TotalTime;currentDay++){
    if(weCanStop) break;
#ifdef FengLeman_DEBUG
    cout<<endl<<"DAY "<<currentDay<<" :"<<endl;
#endif
    //the timer, not important...
    if(currentDay-last1000Day>=1000){
      cout<<"DAY "<<currentDay<<endl;

      if(noMoreDemand)cout<<"no more demand"<<endl;
#ifdef DRAW_IMG
      if(currentDay>200){
        drawImg(currentDay);
      }
#endif
      last1000Day=currentDay;
      time2=clock();
      cout << ((float) (time2 - time1) / CLOCKS_PER_SEC) << " s" << endl;
      time1=time2;
    }


    // build Product Flow Graph (WareHouse--TypeId-->Command)
    // which means, for each demand of a client, we find the nearest warehouse that has the product
    // store in this variable:
    // map<CmdId, vector<pair<WareId, TypeId> > > cmd_mapTo_FlowList
    // for each client(command), we have a list of pair of (warehouseId, productId)
    buildProductFlowGraph(cmd_mapTo_FlowList);
    if(cmd_mapTo_FlowList.empty()){
      noMoreDemand=true;
    }


    // each client(command) propose a list of travels
    // a Travel is defined as (a warehouse, a client, some products)
    // wareHouse----(product1,product2,...)------>client
    giveBestTravels(travels);


    sort(travels.begin(),travels.end()); //in order 1<2<3<4, the best travel is at the end


    // loop for drone
    // each available drone pick a best travel
    // (we will also consider the distance between the drone and the wareHouse)
    // and the choice is stored in drone_travel
    vector<pair<DroneId,Travel> > drone_travel;
    for(auto &drone :drones){
      if(!drone.isBusy){
        dronePickTravel(travels,drone,drone_travel);
      }
    }

    // drones confirm their travel. When it confirm one travel, it will try to load some other product
    // from the same wareHouse to deliver to some clients nearby the target client
    for(auto d_t : drone_travel){
      droneConfirmTravel(drones[d_t.first],d_t.second,nearestClient);
    }

    // we do the simulation, each drone perform its action
    // (taking a new task or count down the time before finishing the current task)
    for(auto &drone : drones){
      drone.makeOneTurn();
    }




    // if all drones are busy, that means for the next turn, no new travels are need to be calculated
    // we just let all drone to count down their task timer

    bool allDroneBusy=true;
    bool allDroneFree=true;
    while(allDroneBusy){
      for(auto &drone : drones){
        if(!drone.isBusy) allDroneBusy=false;
        else allDroneFree=false;
      }
      if(allDroneBusy){
        currentDay++;
#ifdef FengLeman_DEBUG
        cout<<endl<<"DAY busy"<<currentDay<<" :"<<endl;
#endif
        for(auto &drone : drones){
          drone.makeOneTurn();
        }
      }
      if(noMoreDemand && allDroneFree){
        weCanStop=true;
      }
    }
  }
  //cout<<"Score "<<Score<<endl;

  cout<<"Total Score "<<Score<<endl;
  timeEnd=clock();
  cout <<"Total time consumed: "<< ((float) (timeEnd - timeStart) / CLOCKS_PER_SEC) << " s" << endl;
#ifdef WRITE_OUTPUT
  output.close();
#endif
  return 0;
}
