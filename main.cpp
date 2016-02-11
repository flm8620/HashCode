#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <random>
#include <cassert>

#ifndef srcPath
#define srcPath "."
#endif

using namespace std;
//random:
unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 randomGenerator(seed1);

string inputFile = srcPath "/busy_day.in";
int N, M;
int DroneCount;
int TotalTime;
int MaxLoad;
int TypeCount;
int WareCount;
int CmdCount;
typedef int TypeId;
vector<TypeId> TypeWeight;
vector<pair<int,int> > WarePos;
vector<vector<int> > WareProductAmount;

struct Command{
  int x,y;
  int demandAmout;
  vector<int> productList;
};
vector<Command> commands;

void readFile(string fileName, vector<bool> &wall){

}

int main() {
  ifstream input(inputFile);
  input >> N >> M >> DroneCount >> TotalTime >> MaxLoad;
  cout<<N<<M;
  input >> TypeCount;
  for(int i=0;i<TypeCount;i++){
    int weight;
    input>>weight;
    TypeWeight.push_back(weight);
  }
  input >> WareCount;
  for(int i=0;i<WareCount;i++){
    int x,y;
    input >> x >> y;
    WarePos.push_back(make_pair(x,y));
    WareProductAmount.push_back(vector<int>());
    vector<int> &Dispon = WareProductAmount.back();
    for(int j=0;j<TypeCount;j++){
      int numProductDispon;
      input>>numProductDispon;
      Dispon.push_back(numProductDispon);
    }
  }
  input>>CmdCount;
  for(int i=0;i<CmdCount;i++){
    commands.push_back(Command());
    Command &cmd=commands.back();

    input >> cmd.x >> cmd.y;
    input >> cmd.demandAmout;
    for(int k=0;k<cmd.demandAmout;k++){
      TypeId type;
      input>>type;
      cmd.productList.push_back(type);
    }
  }
  cout<<"hello"<<endl;
  return 0;
}
