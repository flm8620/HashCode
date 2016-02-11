#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <random>
#include <cassert>


// Foncteur comparaison
struct Comparepoids {
    bool operator() (const Command &C1, const Command &C2) {
	return  c1.score() <= C2.score();
    }   
};

struct Command {
    int x,y;
    int demandAmout;
    vector<int> productList;
    int poids() {
	int res = 0;
	for (auto it = productList.begin(); it != productList.end(); it++)
	    res += Weight[*it];
	return res;
    }
};

struct Comparedemand {
    bool operator() (const Command &C1, const Command &C2) {
	return  c1.demandAmout() <= C2.demandAmout();
    }   
};


    

// sort command :

sort(commands, Comparepoids);
    



