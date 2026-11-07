#include "parse.h"
#include <cstring>
#include "schedule.h"
using namespace std;
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "provide filename" << endl;
        return -1;
    }

    bool write_ssa = (argc == 3) && (strcmp(argv[2], "--export-ssa") == 0);
    string filename = argv[1];

    vector<struct node> dag = ssa(filename, write_ssa);
    for (const auto &node : dag)
    {
        std::cout << "Operation: " << node.op << ", output: " << node.out << "\n";
    }

    vector<vector<int>> adj_mat = adj_matrix(dag);
    for (const auto &node : adj_mat)
    {
        for (const auto &node2 : node)
        {
            cout << to_string(node2) << " ";
        }
        cout << endl;
    }

    int cycles;
    cout << "ASAP: " << endl;
    map<int, int> asap = ASAP(adj_mat, cycles);
    for (const auto &pair : asap)
    {
        std::cout << dag[pair.first].out << ": " << pair.second << "\n";
    }
    cout << "total cycles: " << cycles << endl
         << endl;

    cout << "ALAP: " << endl;
    map<int, int> alap = ALAP(adj_mat, cycles);
    for (const auto &pair : alap)
    {
        std::cout << dag[pair.first].out << ": " << pair.second << "\n";
    }
    cout << "total cycles: " << cycles << endl
         << endl;

    cout << "force directed schedule: " << endl;
    map<int, int> fds = FDS(adj_mat, cycles, asap, alap);
    for (const auto &pair : fds)
    {
        std::cout << dag[pair.first].out << ": " << pair.second << "\n";
    }
    cout << "total cycles: " << cycles << endl
         << endl;

    cout << "Constrained ASAP: " << endl;
    map<int, int> constraint = {{0, 1}, {1, 1}, {2, 1}};
    map<int, int> constrained_asap = ConstrainedASAP(adj_mat, constraint, cycles, asap, alap);
    for (const auto &pair : constrained_asap)
    {
        std::cout << dag[pair.first].out << ": " << pair.second << "\n";
    }
    cout << "total cycles: " << cycles << endl
         << endl;

    cout << "Constrained ALAP: " << endl;
    map<int, int> constrained_alap = ConstrainedALAP(adj_mat, constraint, cycles, asap, alap);
    for (const auto &pair : constrained_alap)
    {
        std::cout << dag[pair.first].out << ": " << pair.second << "\n";
    }
    cout << "total cycles: " << cycles << endl
         << endl;

    return 0;
}