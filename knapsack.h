#ifndef KNAPSACK_H
#define KNAPSACK_H
#include <set>
#include <vector>
#include <algorithm>

std::multiset<int> knapsack(std::multiset<std::pair<int,int> >& products, int maxLoad);

#endif // KNAPSACK_H
