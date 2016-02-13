#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <random>
#include <cmath>
#include <map>
#include <queue>
#include <functional>   // std::bind
#include <cassert>

#define FLM_DEBUG

#ifndef srcPath
#define srcPath "."
#endif

using namespace std;
//random:
unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 randomGenerator(seed1);

string inputFile = srcPath "/easy_day.in";

int distanceRound(int x1,int y1,int x2,int y2){
  double d=sqrt(double(x1-x2)*double(x1-x2)+double(y1-y2)*double(y1-y2));
  d-=1e-5; // what if sqrt(4) = 2.0000001
  return int(ceil(d));
}

int N, M;
int DroneCount;
int TotalTime;
int MaxLoad;
int TypeCount;
int WareCount;
int CmdCount;
typedef int TypeId;
typedef int CmdId;
typedef int WareId;
vector<int> TypeWeight;

struct WareHouse{
  int id;
  int x,y;
  vector<int> productAvailability;
  void eraseProduct(TypeId id){
    assert(productAvailability[id]>=1);
    productAvailability[id]--;// delete in wareHouse
  }
  void addProduct(TypeId id){
    productAvailability[id]++;// delete in wareHouse
  }
};
vector<WareHouse> wareHouses;

struct Command{
  int id;
  int x,y;
  multiset<TypeId> demands;
  multiset<TypeId> productsOnTheWay;
  bool needUpdate;
  void oneDemandIsOnTheWay(TypeId id){
    auto it = demands.find(id);
    assert(it!=demands.end());
    demands.erase(it); // delete in commands
    productsOnTheWay.insert(id);
    needUpdate=true;
  }
  void productReceived(TypeId id){
    auto it = productsOnTheWay.find(id);
    assert(it!=productsOnTheWay.end());
    productsOnTheWay.erase(it); // delete in commands
    needUpdate=true;
  }

  Command():needUpdate(true){

  }
};
vector<Command> commands;

struct DroneOrder{
  enum DroneOrderType{
    Load, Unload, Deliver, FlyToWareHouse, FlyToClient
  } orderType;
  int wareId;
  int commandId;
  multiset<TypeId> itemsToPickOrDrop;
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
  int wareId;
  int commandId;
  multiset<TypeId> itemsToPickOrDrop;
};

struct Drone{
  int id;
  int x,y;
  queue<DroneOrder> orderQueue;
  DroneTask currentTask;
  multiset<TypeId> loadedProducts;
  bool isDoingATask;
  bool isBusy;
  Drone():isDoingATask(false),isBusy(false){

  }

  int loadedWeight(){
    int sum=0;
    for(auto id : loadedProducts){
      sum+=TypeWeight[id];
    }
    return sum;
  }

  void eraseProduct(TypeId id){
    auto it = loadedProducts.find(id);
    assert(it!=loadedProducts.end());
    loadedProducts.erase(it);
  }
  void addProduct(TypeId id){
    loadedProducts.insert(id);
    assert(this->loadedWeight()<MaxLoad);
  }
  void makeOneTurn(){
    this->isBusy=false;
    if(!this->isDoingATask){
#ifdef FLM_DEBUG
      cout<<"drone "<<this->id<<" just finished it job in last turn."<<endl;
#endif
      this->takeNextOrder_makeItATask();// it may become "Busy" after this call
    }
    if(this->isDoingATask){
      this->isBusy=true;
      this->doTask();
    }
#ifdef FLM_DEBUG
    if(!this->isBusy)
      cout<<"drone "<<this->id<<" has nothing to do now."<<endl;
#endif
  }

  void giveOneOrder(DroneOrder order){
    switch(order.orderType){
      case DroneOrder::Load:
      case DroneOrder::Unload:
      {
        int wid=order.wareId;
        if(this->x==wareHouses[wid].x && this->y==wareHouses[wid].y){
          this->orderQueue.push(order);
        }else{
          DroneOrder gotoPlace;
          gotoPlace.orderType=DroneOrder::FlyToWareHouse;
          gotoPlace.wareId=order.wareId;
          this->orderQueue.push(gotoPlace);
          this->orderQueue.push(order);
        }
        break;
      }
      case DroneOrder::Deliver:
      {
        int cid=order.commandId;
        if(this->x==commands[cid].x && this->y==commands[cid].y){
          this->orderQueue.push(order);
        }else{
          DroneOrder gotoPlace;
          gotoPlace.orderType=DroneOrder::FlyToWareHouse;
          gotoPlace.commandId=order.commandId;
          this->orderQueue.push(gotoPlace);
          this->orderQueue.push(order);
        }
        break;
      }
      case DroneOrder::FlyToClient:
      case DroneOrder::FlyToWareHouse:
        assert(false);
        break;
    }
  }
  void arrivedNextTaskPosition(){
    DroneOrder &nextOrder = orderQueue.front();
    switch(nextOrder.orderType){
      case DroneOrder::Load :
      case DroneOrder::Unload :{
        this->x = wareHouses[nextOrder.wareId].x;
        this->y = wareHouses[nextOrder.wareId].y;
        break;
      }
      case DroneOrder::Deliver:{
        this->x = commands[nextOrder.commandId].x;
        this->y = wareHouses[nextOrder.commandId].y;
        break;
      }
      case DroneOrder::FlyToClient:
      case DroneOrder::FlyToWareHouse:{
        assert(false);
        break;
      }
    }
  }
  void takeNextOrder_makeItATask(){
    if(orderQueue.empty()){
      this->isDoingATask=false;
      return;//because available
    }else{
      this->isDoingATask=true;
    }
    DroneOrder order = orderQueue.front();
    orderQueue.pop();
    switch(order.orderType){
      case DroneOrder::Load:
      {
        assert(x==wareHouses[order.wareId].x && y==wareHouses[order.wareId].y);
        currentTask.taskType=DroneTask::Loading;
        currentTask.wareId=order.wareId;
        currentTask.commandId=order.commandId;
        currentTask.timeToFinish=1;
        currentTask.itemsToPickOrDrop=order.itemsToPickOrDrop;
#ifdef FLM_DEBUG
        cout<<"drone "<<this->id<<" is starting a new task : Loading at WareHouse "
           <<order.wareId<<endl;
#endif
        break;
      }
      case DroneOrder::Unload:
      {
        assert(x==wareHouses[order.wareId].x && y==wareHouses[order.wareId].y);
        currentTask.taskType=DroneTask::Unloading;
        currentTask.wareId=order.wareId;
        currentTask.timeToFinish=1;
        currentTask.itemsToPickOrDrop=order.itemsToPickOrDrop;
#ifdef FLM_DEBUG
        cout<<"drone "<<this->id<<" is starting a new task : Unloading at WareHouse "
           <<order.wareId<<endl;
#endif
        break;
      }
      case DroneOrder::Deliver:
      {
        assert(x==commands[order.commandId].x && y==commands[order.commandId].y);
        currentTask.taskType=DroneTask::DropingToClient;
        currentTask.commandId=order.commandId;
        currentTask.timeToFinish=1;
        currentTask.itemsToPickOrDrop=order.itemsToPickOrDrop;
#ifdef FLM_DEBUG
        cout<<"drone "<<this->id<<" is starting a new task : Deliver at Client of command "
           <<order.commandId<<endl;
#endif
        break;
      }
      case DroneOrder::FlyToClient:{
        currentTask.commandId=order.commandId;
        Command &cmd = commands[order.commandId];
        currentTask.timeToFinish=distanceRound(x,y,cmd.x,cmd.y);
#ifdef FLM_DEBUG
        cout<<"drone "<<this->id<<" is starting a new task : FlyToClient of command "
           <<order.commandId<<endl;
#endif
        break;
      }
      case DroneOrder::FlyToWareHouse:
        currentTask.wareId=order.wareId;
        WareHouse &w = wareHouses[order.wareId];
        currentTask.timeToFinish=distanceRound(x,y,w.x,w.y);
#ifdef FLM_DEBUG
        cout<<"drone "<<this->id<<" is starting a new task : FlyTo WareHouse "
           <<order.wareId<<endl;
#endif
        break;
    }
  }
  void doTask(){
    switch (currentTask.taskType){
      case DroneTask::Loading :{
        for(auto typeId : currentTask.itemsToPickOrDrop){
          wareHouses[currentTask.wareId].eraseProduct(typeId);
          commands[currentTask.commandId].oneDemandIsOnTheWay(typeId);
          this->addProduct(typeId);
        }
        this->isDoingATask=false;
        break;
      }
      case DroneTask::Unloading :{
        loadedProducts=currentTask.itemsToPickOrDrop;
        for(auto typeId : currentTask.itemsToPickOrDrop){
          this->eraseProduct(typeId);
          wareHouses[currentTask.wareId].addProduct(typeId);
        }
        this->isDoingATask=false;
        break;
      }
      case DroneTask::DropingToClient:{
        loadedProducts=currentTask.itemsToPickOrDrop;
        for(auto typeId : currentTask.itemsToPickOrDrop){
          this->eraseProduct(typeId);
          commands[currentTask.commandId].productReceived(typeId);
        }
        this->isDoingATask=false;
        break;
      }
      case DroneTask::WaytoClient:
      case DroneTask::WaytoWareHouse:{
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
  double commandFinishPercentage;
  int distance;
  bool operator <(const Travel & other){
    return score<other.score;
  }
};




void readFile(string inputFile){
  ifstream input(inputFile);
  input >> N >> M >> DroneCount >> TotalTime >> MaxLoad;
  input >> TypeCount;
  for(int i=0;i<TypeCount;i++){
    int weight;
    input>>weight;
    TypeWeight.push_back(weight);
  }
  input >> WareCount;
  for(int i=0;i<WareCount;i++){
    WareHouse wareHouse;
    wareHouse.id=i;
    input >> wareHouse.x >> wareHouse.y;
    for(int j=0;j<TypeCount;j++){
      int numProductAvailable;
      input>>numProductAvailable;
      wareHouse.productAvailability.push_back(numProductAvailable);
    }
    wareHouses.push_back(wareHouse);
  }
  input>>CmdCount;
  for(int i=0;i<CmdCount;i++){
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
    commands.push_back(cmd);
  }
}

void initializeDrone(){
  for(int i=0;i<DroneCount;i++){
    Drone drone;
    drone.id=i;
    drone.x=wareHouses[0].x;
    drone.y=wareHouses[0].y;
    drones.push_back(drone);
  }
}

bool CmdIsNearerToTarget(CmdId id1,CmdId id2,CmdId targetId){
  int d1=distanceRound(commands[id1].x,commands[id1].y,commands[targetId].x,commands[targetId].y);
  int d2=distanceRound(commands[id2].x,commands[id2].y,commands[targetId].x,commands[targetId].y);
  return d1<d2;
}
const int NearestCmdCount=5;
map<CmdId,vector<CmdId> > buildNearestCmdsMap(){
  using namespace placeholders;
  map<CmdId,vector<CmdId> > cmd_mapto_nearestCmds;
  vector<CmdId> cmdIDs;
  for(int i=0;i<CmdCount;i++){
    cmdIDs.push_back(i);
  }
  for(int cmdId=0;cmdId<CmdCount;cmdId++){
    auto compareFun = bind(CmdIsNearerToTarget,_1,_2,cmdId);
    partial_sort(cmdIDs.begin(),cmdIDs.begin()+5,cmdIDs.end(),compareFun);
    for(int k=0;k<NearestCmdCount;k++){
      cmd_mapto_nearestCmds[cmdId].push_back(cmdIDs[k]);
    }
  }
}


multiset<TypeId> first_fit(multiset<TypeId>& products){
  multiset<TypeId> result;
  vector<TypeId> vec;
  copy(products.begin(),products.end(),back_inserter(vec));
  sort(vec.begin(),vec.end(),compareByWeight);
  int weightSum=0;
  for(auto id : vec){
    if(weightSum+TypeWeight[id]<200){
      weightSum+=TypeWeight[id];
      result.insert(id);
      auto it=products.find(id);
      products.erase(it);
    }else{
      break;
    }
  }
  return result;
}

void buildProductFlowGraph(map<CmdId, vector<pair<WareId, TypeId> > >&cmd_mapTo_FlowList)
{
  vector<WareHouse> wareHousesCopy=wareHouses;
  for(int k=0;k<CmdCount;k++){
    Command &cmd = commands[k];
    if(!cmd.needUpdate){
      continue;
    }
    else{
      cmd_mapTo_FlowList[k].clear();
    }
    for(auto itemId : cmd.demands){
      int minDistance=10000000;
      int ChosenWareId=-1;
      for(int i=0;i<WareCount;i++){
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
      wareHousesCopy[ChosenWareId].productAvailability[itemId]--;
      cmd_mapTo_FlowList[k].push_back(make_pair(ChosenWareId,itemId));
    }
  }
}

map<CmdId, vector<pair<WareId, TypeId> > >cmd_mapTo_FlowList;

void giveBestTravels(vector<Travel>& travels)
{
  travels.clear();
  for(int cmdId=0;cmdId<CmdCount;cmdId++){
    //find where does each product come from
    set<WareId> ChosenWareHouses;
    map<WareId, multiset<TypeId> > ware_map_TypeSet;
    vector<pair<WareId, TypeId> > &flowList=cmd_mapTo_FlowList[cmdId];
    for(auto arrow : flowList){
      int wareId=arrow.first;
      int typeId=arrow.second;
      ChosenWareHouses.insert(wareId);
      ware_map_TypeSet[wareId].insert(typeId);
    }
    //from a certain wareHouse, we find a best combination of products
    //use FIRST_FIT algo
    for(auto wareId : ChosenWareHouses){
      multiset<TypeId> &products =  ware_map_TypeSet[wareId];
      multiset<TypeId> loadProduct=first_fit(products);
      Travel travel;
      travel.commandId=cmdId;
      travel.wareId=wareId;
      travel.distance = distanceRound(commands[cmdId].x,commands[cmdId].y,
                                      wareHouses[wareId].x,wareHouses[wareId].y);
      travel.commandFinishPercentage=double(loadProduct.size())/flowList.size();
      travel.score=travel.commandFinishPercentage/travel.distance;
      for(auto item : loadProduct)
        travel.productsToDeliver.insert(item);
      travels.push_back(travel);
    }
  }
}

int main() {
  readFile(inputFile);
  initializeDrone();
  //map<CmdId,vector<CmdId> > cmd_mapto_nearestCmds;

  //build product flow graph




  vector<Travel> travels;
  vector<DroneTask> orders;
  for(int t=0;t<TotalTime;t++){
#ifdef FLM_DEBUG
    cout<<"DAY "<<t<<" :"<<endl;
#endif
    //build Product Flow Graph (WareHouse--TypeId-->Command)
    buildProductFlowGraph(cmd_mapTo_FlowList);
    //each cmd give a best travel
    giveBestTravels(travels);

    sort(travels.begin(),travels.end()); //1<2<3<4
    //loop for drone
    //each available drone pick a travel
    for(auto &drone : drones){
      if(!drone.isBusy){
        sort(travels.begin(),travels.end(),[drone](const Travel& t1,const Travel& t2){
          WareHouse& w1=wareHouses[t1.wareId];
          WareHouse& w2=wareHouses[t2.wareId];
          int distanceGoto1 = distanceRound(drone.x,drone.y,w1.x,w1.y);
          int distanceGoto2 = distanceRound(drone.x,drone.y,w2.x,w2.y);
          double score1 = t1.commandFinishPercentage/(t1.distance+distanceGoto1);
          double score2 = t2.commandFinishPercentage/(t2.distance+distanceGoto2);
          return score1<score2;
        });
        Travel bestTravel = travels.back();
        travels.pop_back();
        DroneOrder orderLoad;
        orderLoad.orderType=DroneOrder::Load;
        orderLoad.wareId=bestTravel.wareId;
        orderLoad.commandId=bestTravel.commandId;
        orderLoad.itemsToPickOrDrop=bestTravel.productsToDeliver;
        drone.giveOneOrder(orderLoad);
        DroneOrder orderDeliver;
        orderDeliver.orderType=DroneOrder::Deliver;
        orderDeliver.commandId=bestTravel.commandId;
        orderDeliver.itemsToPickOrDrop=bestTravel.productsToDeliver;
        drone.giveOneOrder(orderDeliver);
#ifdef FLM_DEBUG
        cout<<"drone "<<drone.id<<"\tget new job :"<<endl;
        cout<<"\tFrom ware"<<bestTravel.wareId<<" to command"<<bestTravel.commandId<<endl;
        cout<<"Items:\t";
        for(auto item : bestTravel.productsToDeliver){
          cout<<item<<'\t';
        }
        cout<<endl;
#endif
      }
    }
    for(auto &drone : drones){
      drone.makeOneTurn();
    }
  }
  return 0;
}
