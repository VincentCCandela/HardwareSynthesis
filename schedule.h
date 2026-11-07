#ifndef SCHEDULE
#define SCHEDULE
#include "parse.h"
#include <map>

map<int, int> ASAP(vector<vector<int>> adj_mat, int &total_cycle);
vector<vector<int>> adj_matrix(vector<struct node> dag);
map<int, int> ALAP(vector<vector<int>> adj_mat, int total_cycle);
map<int, int> ConstrainedASAP(vector<vector<int>> adj_mat, map<int, int> constraint, int &total_cycle, map<int, int> asap, map<int, int> alap);
map<int, int> ConstrainedALAP(vector<vector<int>> adj_mat, map<int, int> constraint, int &total_cycle, map<int, int> asap, map<int, int> alap);
map<int, int> FDS(vector<vector<int>> adj_mat, int &total_cycle, map<int, int> asap, map<int, int> alap);
#endif