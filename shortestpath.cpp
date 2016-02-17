// Leman FENG

#include "shortestpath.h"

#include <vector>
#include <iostream>
#include <set>
#include <cassert>
#include <algorithm>
//#include <chrono>
#include <random>

using namespace std;
unsigned seed1 = 11235813;
std::mt19937 gen(seed1);

static void randomPermutation(vector<int> & v){
  int N=v.size();
  std::uniform_int_distribution<int> randInt(0,N-1);
  for(int i=0;i<v.size();i++){
    swap(v[i],v[randInt(gen)]);
  }
}

static int pathLength(vector<int>& order,vector<int>& distanceMatix,vector<int> ware_cmd_distance){
  //distance of ware -> 0 -> 1 -> 2 -> ... -> N-1 -> ware
  int d=0;
  d+=ware_cmd_distance[order[0]];
  int N=order.size();
  assert(distanceMatix.size()==N*N);
  for(int i=0;i<order.size()-1;i++){
    d+=distanceMatix[order[i]*N+order[i+1]];
  }
  d+=ware_cmd_distance[order[N-1]];
  return d;
}

vector<int> bestVisitOrder(vector<int>& distanceMatix, vector<int> ware_cmd_distance){
  //local search:
  // start=0..N-2, end =1..N-1, start<end
  // example
  // start=3, end=6

  // 0 1 2 3 4 5 6 7 8 9
  //       ^     ^
  //       _______
  // 0 1 2 6 5 4 3 7 8 9


  int N=ware_cmd_distance.size();
  assert(distanceMatix.size()==N*N);
  assert(ware_cmd_distance.size()==N);
  vector<int> order(N);
  for(int i=0;i<N;i++)order[i]=i;
  randomPermutation(order);

  const int Iter=30;
  int orderDistance=pathLength(order,distanceMatix,ware_cmd_distance);
  int currentBest=orderDistance;

  for(int k=0;k<Iter;k++){
    int minDis=100000000;
    vector<int> bestNeighbor;
    for(int start=0;start<N-1;start++){
      for(int end=start;end<N;end++){
        vector<int> newOrder=order;
        reverse(newOrder.begin()+start,newOrder.begin()+end+1);
        int dist=pathLength(newOrder,distanceMatix,ware_cmd_distance);
        if(dist<minDis){
          minDis=dist;
          bestNeighbor=newOrder;
        }
      }
    }
    if(minDis>=currentBest) // maybe local min
      break;
    else{
      order=bestNeighbor;
      currentBest=minDis;
    }
  }
  return order;
}
