#ifndef DATAPATH
#define DATAPATH
#include "schedule.h"
struct reg
{
    int in;          // input node index
    int out;         // output node index
    int enable_time; // enable at the start of which cycle
};

struct mux
{
    map<int, int> in; // input nodes index, cycle used
    int output;       // output to which resource
};

vector<struct reg> registers(vector<vector<int>> adj_matrix, map<int, int> schedule);
vector<vector<struct mux>> multiplexers(vector<vector<int>> adj_matrix, map<int, int> schedule);
void sequencer(int cycle, vector<vector<struct mux>> mux_list, vector<struct reg> reg_list, vector<int> &active_reg, vector<map<int, int>> &active_mux);
#endif