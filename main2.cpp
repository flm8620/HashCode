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
vector<pair<int, int> > WarePos;
vector<vector<int> > WareProductAmount;

struct Command {
	int x, y;
	int demandAmout;
	vector<int> productList;
	int poids() {
		int res = 0;
		for (auto it = productList.begin(); it != productList.end(); it++)
			res += TypeWeight[*it];
		return res;
	}
};

vector<Command> commands;

// Foncteur comparaison
struct Comparepoids {
	bool operator()(const Command &C1, const Command &C2) {
		return (C1.poids() <= C2.poids());
	}
};

struct Comparedemand {
	bool operator()(const Command &C1, const Command &C2) {
		return C1.demandAmout() <= C2.demandAmout();
	}
};

void readFile(string fileName, vector<bool> &wall) {

}

int main() {

	ifstream input(inputFile);
	input >> N >> M >> DroneCount >> TotalTime >> MaxLoad;
	input >> TypeCount;
	for (int i = 0; i < TypeCount; i++) {
		int weight;
		input >> weight;
		TypeWeight.push_back(weight);
	}
	input >> WareCount;
	for (int i = 0; i < WareCount; i++) {
		int x, y;
		input >> x >> y;
		WarePos.push_back(make_pair(x, y));
		WareProductAmount.push_back(vector<int>());
		vector<int> &Dispon = WareProductAmount.back();
		for (int j = 0; j < TypeCount; j++) {
			int numProductDispon;
			input >> numProductDispon;
			Dispon.push_back(numProductDispon);
		}
	}
	input >> CmdCount;
	for (int i = 0; i < CmdCount; i++) {
		commands.push_back(Command());
		Command &cmd = commands.back();

		input >> cmd.x >> cmd.y;
		input >> cmd.demandAmout;
		for (int k = 0; k < cmd.demandAmout; k++) {
			TypeId type;
			input >> type;
			cmd.productList.push_back(type);
		}
	}

	// So Fujikake

	sort(commands, Comparepoids);

	map<Command, vector<pair<int, int> > > mapCmdWareType;

	vector<int> custmerOrder; // given
	int waresize = WarePos.size();

	for (vector<Command>::iterator it = custmerOrder.begin();
			it != custmerOrder.end(); ++it) {

		vector<int> tp = (*it).productList;

		for (vector<int>::iterator it2 = tp.begin(); it2 != tp.end(); ++it2) {

			int wareid = 0;
			float x_before = 100000000000;
			float x_after = 100000000000;

			vector<int>::iterator target_index;

			for (int i = 0; i < waresize; ++i) {
				vector<int>::iterator target = find(
						WareProductAmount.at(i).begin(),
						WareProductAmount.at(i).end(), *it2);
				if (target != WareProductAmount.at(i).end()) {

					x_after = sqrt(
							pow((WarePos.at(i).first - (*it).x), 2)
									+ pow((WarePos.at(i).second - (*it).y), 2));
					if (x_after < x_before) {
						wareid = i;
						x_before = x_after;
					}

				}
				target_index = find(WareProductAmount.at(wareid).begin(),
						WareProductAmount.at(wareid).end(), *it2);
				if (mapCmdWareType.find(*it) != mapCmdWareType.end()) {
					vector<pair<int, int> > tv;
					tv.push_back(make_pair(wareid, *it2));
					mapCmdWareType[*it] = tv;
				} else {
					mapCmdWareType[*it].push_back(make_pair(wareid, *it2));
				}
			}
		}
	}

	return 0;
}

