#include "parse.h"
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
    return 0;
}