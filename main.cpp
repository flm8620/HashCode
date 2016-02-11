#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
//random value generator:
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

string inputFile = srcPath "/right_angle.in";
int N, M;


void readFile(string fileName, vector<bool> &wall);
int minOper(int N,int M,int knowMin);
set<pair<int,int> > getTargets(vector<bool> & wall);
pair<int,int> pickRandomTarget(set<pair<int,int> >& targets);
void randomPermutation(vector<int> &v){
  int n=v.size();
  std::uniform_int_distribution<int> randomNumber(0, n-1);
  for(int i=0;i<n;i++){
    swap(v[i],v[randomNumber(randomGenerator)]);
  }
}

bool allTrue(vector<bool> & wall, int top, int left, int bottom, int right){
  for(int i=top;i<=bottom;i++){
    for(int j=left;j<=right;j++){
      if(!wall[i*M+j]) return false;
    }
  }
  return true;
}

void spreadRectangle(vector<bool>& wall,pair<int,int> startPoint,int &left,int& right,int& top,int& bottom){
  top=bottom=startPoint.first;
  left=right=startPoint.second;
  assert(wall[top*M+left]);
  vector<int> direction = {1,2,3,4};// left right up down
  randomPermutation(direction);
  for(int k=0;k<4;k++){
    if(direction[k]==1){
      while(allTrue(wall,top,left-1,bottom,left-1)){
        left-=1;
      }
    }else if(direction[k]==2){
      while(allTrue(wall,top,right+1,bottom,right+1)){
        right+=1;
      }
    }else if(direction[k]==3){
      while(allTrue(wall,top-1,left,top-1,right)){
        top-=1;
      }
    }else if(direction[k]==4){
      while(allTrue(wall,bottom+1,left,bottom+1,right)){
        top-=1;
      }
    }else{
      assert(false);
    }
  }
}

void eraseRectangle(vector<bool> &wall,set<pair<int,int> > &targets,int left,int right,int top,int bottom){
  for(int i=top;i<=bottom;i++){
    for(int j=left;j<=right;j++){
      assert(wall[i*M+j]);
      wall[i*M+j]=false;
      auto it=targets.find(make_pair(i,j));
      assert(it!=targets.end());
      targets.erase(it);
    }
  }
}

int main() {
  vector<bool> wall;
  readFile(inputFile,wall);
  set<pair<int,int> > targets=getTargets(wall);
  //auto coord=pickRandomTarget(targets);
  //cout<<coord.first<<' '<<coord.second<<endl;
  //int left,right,top,bottom;
  //spreadRectangle(wall,coord,left,right,top,bottom);
  //cout<<left<<' '<<right<<' '<<top<<' '<<bottom<<endl;

  vector<bool> wallToPaint=wall;
  set<pair<int,int> > targetsLeft=targets;
  int operationCount=0;
  while(targetsLeft.size()!=0){
    auto coord=pickRandomTarget(targetsLeft);
    int left,right,top,bottom;
    spreadRectangle(wall,coord,left,right,top,bottom);
    cout<<left<<' '<<right<<' '<<top<<' '<<bottom<<endl;
    eraseRectangle(wallToPaint,targetsLeft,left,right,top,bottom);
    operationCount+=minOper(right-left+1,bottom-top+1,100000000);
  }
  cout<<operationCount<<endl;

  //cout<<minOper(6,10,100)<<endl;
  //  for (int i = 0; i < n; i++) {
  //    for (int j = 0; j < m; j++) {
  //      if (wall[i * m + j])
  //        cout << '#';
  //      else
  //        cout << '.';
  //    }
  //    cout << endl;
  //  }
  return 0;
}

void readFile(string fileName, vector<bool> &wall) {
  ifstream input(fileName);
  input >> N >> M;
  wall.resize(N * M);
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      char c;
      input >> c;
      if (c == '.') {
        wall[i * M + j] = false;
      } else {
        wall[i * M + j] = true;
      }
    }
  }
}

set<pair<int,int> > getTargets(vector<bool> &wall){
  set<pair<int,int> > targets;
  for(int i=0;i<N;i++){
    for(int j=0;j<M;j++){
      if(wall[i*M+j])
        targets.insert(make_pair(i,j));
    }
  }
  return targets;
}

pair<int,int> pickRandomTarget(set<pair<int,int> >& targets){
  int n=targets.size();
  std::uniform_int_distribution<int> randomNumber(0, n-1);
  int index=randomNumber(randomGenerator);
  auto it=targets.begin();
  advance(it,index);
  return *it;
}

int minOper(int n,int m,int knowMin){
  /*    #####
   *    #####
   *    #####
   *    n=3, m=5
   */
  if(n>m)
    swap(n,m);
  assert(n>=0 && m>=0);
  if(n==0)
    return 0;
  if(n<=2){
    if(knowMin<=n)
      return knowMin;
    else{
      cout<<"n="<<n<<", draw lines"<<endl;
      return n;
    }
  }
  if(n%2==1){
    int drawSmallOper=minOper(m-n,n,n-1);
    int drawSquareFirst=drawSmallOper+1;
    if(knowMin<=drawSquareFirst && knowMin<=n)
      return knowMin;
    if(n<=drawSquareFirst){
      cout<<"draw "<<n<<" lines"<<endl;
      return n;
    }else{
      cout<<"draw a square "<<n<<"x"<<n<<endl;
      return drawSquareFirst;
    }
  }else{
    int drawSmallOper=minOper(m-(n-1),n-1,n-2);
    int drawSquareAndALineFirst=drawSmallOper+2;
    if(knowMin<=drawSquareAndALineFirst || knowMin<=n)
      return knowMin;
    if(n<=drawSquareAndALineFirst){
      cout<<"draw "<<n<<" lines"<<endl;
      return n;
    }else{
      cout<<"draw a square "<<n-1<<"x"<<n-1<<" and a line in 2 steps"<<endl;
      return drawSquareAndALineFirst;
    }
  }
}
