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
vector<int> TypeWeight;
vector<pair<int,int> > WarePos;
vector<vector<int> > WareProductAmount;
struct Command{
  int x,y;
  int demandAmout;
  vector<int> productList;
};


void readFile(string fileName, vector<bool> &wall){

}

int main() {

  return 0;
}
