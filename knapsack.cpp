
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include "knapsack.h"
using namespace std;

//动态规划求解
void zero_one_pack(int total_weight, int w[], int v[], int flag[], int n) {
  int *c = new int[(n+1)*(total_weight+1)];
  for(int i=0;i<(n+1)*(total_weight+1);i++)c[i]=0;
  //int c[MAX_NUM+1][MAX_WEIGHT+1] = {0}; //c[i][j]表示前i个物体放入容量为j的背包获得的最大价值
  // c[i][j] = max{c[i-1][j], c[i-1][j-w[i]]+v[i]}
  //第i件物品要么放，要么不放
  //如果第i件物品不放的话,就相当于求前i-1件物体放入容量为j的背包获得的最大价值
  //如果第i件物品放进去的话,就相当于求前i-1件物体放入容量为j-w[i]的背包获得的最大价值
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= total_weight; j++) {
      if (w[i] > j) {
        // 说明第i件物品大于背包的重量，放不进去
        c[i*(total_weight+1)+j] = c[(i-1)*(total_weight+1)+j];
      } else {
        //说明第i件物品的重量小于背包的重量，所以可以选择第i件物品放还是不放
          if (c[(i-1)*(total_weight+1)+j] > v[i]+c[(i-1)*(total_weight+1)+(j-w[i])]) {
            c[i*(total_weight+1)+j] = c[(i-1)*(total_weight+1)+j];
          }
          else {
            c[i*(total_weight+1)+j] =  v[i] + c[(i-1)*(total_weight+1)+j-w[i]];
          }
      }
    }
  }

  //下面求解哪个物品应该放进背包
  int i = n, j = total_weight;
  while (c[i*(total_weight+1)+j] != 0) {
    if (c[(i-1)*(total_weight+1)+j-w[i]]+v[i] == c[i*(total_weight+1)+j]) {
      // 如果第i个物体在背包，那么显然去掉这个物品之后，前面i-1个物体在重量为j-w[i]的背包下价值是最大的
      flag[i] = 1;
      j -= w[i];

    }
    --i;
  }
  delete c;

}

//回溯法求解
std::multiset<int> knapsack(std::multiset<std::pair<int,int> >& products,int maxLoad){
  int N=products.size();
  int *w = new int[N+1];
  int *v = new int[N+1];
  int *IDs = new int[N+1];
  w[0]=0;v[0]=0;
  int i=1;
  for(auto p : products){
    w[i]=v[i]=p.second;
    IDs[i]=p.first;
    i++;
  }
  int *flag= new int[N+1]; //flag[i][j]表示在容量为j的时候是否将第i件物品放入背包
  for (int i = 0; i < N+1; i++) flag[i]=0;
  zero_one_pack(maxLoad, w, v, flag, N);
  //cout << "需要放入的物品如下" << endl;
  multiset<int> sol;
  for (int i = 1; i <= N; i++) {
    if (flag[i] == 1)
      sol.insert(IDs[i]);
      //cout << i << "重量为" << w[i] << ", 价值为" << v[i] << endl;
  }
  //cout << "总的价值为: " << total_value << endl;
  delete w;
  delete v;
  delete flag;
  delete IDs;
  return sol;
}
