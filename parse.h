#ifndef PARSE
#define PARSE
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
using namespace std;

// two input node to represent dependency
struct node
{
    string in1;
    string in2;
    string out;
    string op;
};
int extract_variable(string line, string &lhs, string &in1, string &in2, string &op);
vector<struct node> ssa(string filename, bool write, vector<int> &resource);
#endif