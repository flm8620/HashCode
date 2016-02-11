/*
 * createMap.cpp
 *
 *  Created on: 2016/02/11
 *      Author: sofujikake
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <random>
#include <cassert>
#include <map>

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
vector<pair<int, int> > WarePos;
vector<vector<int> > WareProductAmount;

int test() {

	map<int, vector<pair<int, int> > > mapWareCustProd;

	vector<int> custmerOrder; // given
	map<int, vector<int> > mapCustProd; // given
	map<int, vector<int> > mapWareProd; // given
	vector<int> wares; // given

	for (vector<int>::iterator it = custmerOrder.begin();
			it != custmerOrder.end(); ++it) {
		vector<int> tp = mapCustProd[*it];
		for (vector<int>::iterator it2 = tp.begin(); it2 != tp.end(); ++it2) {
			int wareid = 0;
			float x_before = 100000000000;
			float x_after = 100000000000;
			for (vector<int>::iterator it3 = wares.begin(); it3 != wares.end();
					++it3) {
				vector<int>::iterator target = find(mapWareProd[*it3].begin(),
						mapWareProd[*it3].end(), *it2);
				if (target != mapWareProd[*it3].end()) {
					x_after = 10; // to be changed
					if (x_after < x_before) {
						wareid = *it3;
						x_before = x_after;
					}
				}
			}
			mapWareCustProd[wareid] = pair<int, int>(*it, *it2);
		}
	}
	return 0;
}
