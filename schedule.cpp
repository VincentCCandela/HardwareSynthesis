#include "schedule.h"
#include <cmath>
#include <limits>
#include <set>

using namespace std;

map<string, int> op_map = {
    {"+", 0},
    {"-", 1},
    {"*", 2}};

// construct adjacency matrix representation of DAG
vector<vector<int>> adj_matrix(vector<struct node> dag)
{

    vector<vector<int>> adj_mat;

    size_t i, j;
    size_t n = dag.size();
    for (i = 0; i < n; i++)
    {
        // a_ij = 1 means there exist an edge from i to j
        vector<int> row;
        for (j = 0; j < n; j++)
        {
            if (i != j && (dag[i].out == dag[j].in1 || dag[i].out == dag[j].in2))
            {
                row.push_back(1);
            }
            // use the a_ii to store operation type
            else if (i == j)
            {
                row.push_back(op_map[dag[i].op]);
            }
            else
            {
                row.push_back(0);
            }
        }
        adj_mat.push_back(row);
    }
    return adj_mat;
}

// return the column of a matrix given an index
vector<int> get_col(vector<vector<int>> mat, int index)
{
    vector<int> col;
    size_t i, j;
    size_t n = mat.size();
    for (i = 0; i < n; i++)
    {
        col.push_back(mat[i][index]);
    }
    return col;
}

// schedule assigns each node to a time
map<int, int> ASAP(vector<vector<int>> adj_mat, int &total_cycle)
{
    map<int, int> schedule; // maps node index to cycle
    int cycle = 0;          // current cycle

    size_t i, j;
    size_t n = adj_mat.size();

    vector<int> ready_nodes;

    bool ready;
    while (schedule.size() < n)
    {
        for (j = 0; j < n; j++)
        {
            vector<int> col = get_col(adj_mat, j);
            ready = true; // whether all dependencies are resolved
            for (i = 0; i < n; i++)
            {

                // node i not already scheduled and j has a dependency on i
                if (i != j && schedule.count(i) == 0 && col[i] == 1)
                {
                    ready = false;
                }
            }
            if (ready)
            {
                ready_nodes.push_back(j);
            }
        }
        for (const auto &node : ready_nodes)
        {
            // add only the new nodes
            if (schedule.count(node) == 0)
            {
                schedule[node] = cycle;
            }
        }
        cycle++;
        ready_nodes.clear();
    }
    total_cycle = cycle;

    return schedule;
}

map<int, int> ALAP(vector<vector<int>> adj_mat, int total_cycle)
{
    map<int, int> schedule;      // maps node index to cycle
    int cycle = total_cycle - 1; // start from final stage and search forward

    size_t i, j;
    size_t n = adj_mat.size();
    bool ready = true;   // dependencies with later nodes
    bool allowed = true; // dependencies with earlier nodes
    set<int> previous;   // previously evaluated nodes

    // output nodes are those that don't point to anyone
    for (i = 0; i < n; i++)
    {
        ready = true;
        for (j = 0; j < n; j++)
        {
            if (adj_mat[i][j] == 1 && i != j)
            {
                ready = false;
            }
        }
        if (ready)
        {
            // start from final cycle on critical path
            schedule[i] = cycle;
            previous.emplace(i);
        }
    }
    cycle--;

    while (schedule.size() < n)
    {
        // cout << schedule.size() << endl;
        for (i = 0; i < n; i++)
        {
            ready = false;
            allowed = true;
            for (j = 0; j < n; j++)
            {
                // current node point to one of previously evaluated nodes
                if (i != j && previous.find(j) != previous.end() && adj_mat[i][j] == 1)
                {
                    ready = true;
                }
                // current node is dependency of nodes earlier in time
                if (i != j && adj_mat[i][j] == 1 && (schedule.count(j) == 0 || schedule[j] <= cycle))
                {
                    allowed = false;
                }
            }
            if (ready && allowed)
            {
                schedule[i] = cycle;
            }
        }
        previous.clear();
        for (i = 0; i < n; i++)
        {
            // nodes that are just added
            if (schedule.count(i) > 0 && schedule[i] == cycle)
            {
                previous.emplace(i);
            }
        }
        cycle--;
    }

    return schedule;
}

// slack = alap time - asap time
map<int, int> calc_slack(map<int, int> asap, map<int, int> alap)
{
    map<int, int> slack;
    size_t i;
    for (i = 0; i < asap.size(); i++)
    {
        slack[i] = alap[i] - asap[i];
    }
    return slack;
}

// kick out top k elements of ready_nodes according to slack and add to a vector
vector<int> get_top(map<int, int> slack, int k, set<int> &ready_nodes)
{
    vector<int> critical;
    size_t i, j;
    size_t n = slack.size();
    int critical_node = *ready_nodes.begin();
    if (ready_nodes.size() < k)
    {
        k = ready_nodes.size();
        for (const auto &node : ready_nodes)
        {
            critical.push_back(node);
        }
        ready_nodes.clear();
        return critical;
    }
    for (i = 0; i < k; i++)
    {
        for (const auto &node : ready_nodes)
        {
            if (slack[node] < slack[critical_node])
            {
                critical_node = node;
            }
        }
        ready_nodes.erase(critical_node);
        critical.push_back(critical_node);
        critical_node = *ready_nodes.begin();
    }
    return critical;
}

// constraint is a map with pairs (operation resource, available unit)
map<int, int> ConstrainedASAP(vector<vector<int>> adj_mat, vector<int> constraint, int &total_cycle, map<int, int> asap, map<int, int> alap)
{
    map<int, int> schedule; // maps node index to cycle
    int cycle = 0;          // current cycle

    size_t i, j;
    size_t n = adj_mat.size();

    map<int, int> slack = calc_slack(asap, alap);

    set<int> ready_nodes_add;
    set<int> ready_nodes_sub;
    set<int> ready_nodes_mult;
    bool ready;
    while (schedule.size() < n)
    {
        // cout << schedule.size() << endl;
        for (j = 0; j < n; j++)
        {
            vector<int> col = get_col(adj_mat, j);
            ready = true; // whether all dependencies are resolved
            for (i = 0; i < n; i++)
            {
                // node i not already scheduled and j has a dependency on i
                if (i != j && schedule.count(i) == 0 && col[i] == 1)
                {
                    ready = false;
                }
            }
            // filter out already scheduled nodes
            if (ready && schedule.count(j) == 0)
            {
                if (adj_mat[j][j] == 0 && ready_nodes_add.find(j) == ready_nodes_add.end())
                    ready_nodes_add.emplace(j);
                else if (adj_mat[j][j] == 1 && ready_nodes_sub.find(j) == ready_nodes_sub.end())
                    ready_nodes_sub.emplace(j);
                else if (adj_mat[j][j] == 2 && ready_nodes_mult.find(j) == ready_nodes_mult.end())
                    ready_nodes_mult.emplace(j);
            }
        }

        vector<int> next_add = get_top(slack, constraint[0], ready_nodes_add);
        vector<int> next_sub = get_top(slack, constraint[1], ready_nodes_sub);
        vector<int> next_mult = get_top(slack, constraint[2], ready_nodes_mult);
        if (next_add.empty() && next_sub.empty() && next_mult.empty())
        {
            cout << "cannot schedule" << endl;
            break;
        }

        // get the most critical nodes according to resource available
        // cout << "ready additions: ";

        for (const auto &node : next_add)
        {
            // cout << node << " ";
            ready_nodes_add.erase(node);
            schedule[node] = cycle;
        }
        // cout << endl;

        // cout << "ready sub: ";

        for (const auto &node : next_sub)
        {
            // cout << node << " ";
            ready_nodes_sub.erase(node);
            schedule[node] = cycle;
        }
        // cout << endl;

        // cout << "ready mult: ";
        for (const auto &node : next_mult)
        {
            // cout << node << " ";
            ready_nodes_mult.erase(node);
            schedule[node] = cycle;
        }
        // cout << endl;

        cycle++;
    }
    total_cycle = cycle;
    return schedule;
}

map<int, int> ConstrainedALAP(vector<vector<int>> adj_mat, vector<int> constraint, int &total_cycle, map<int, int> asap, map<int, int> alap)
{
    map<int, int> schedule; // maps node index to cycle

    int cycle = 19; // worst case: one operation per cycle

    size_t i, j;
    size_t n = adj_mat.size();

    map<int, int> slack = calc_slack(asap, alap);

    set<int> ready_nodes_add;
    set<int> ready_nodes_sub;
    set<int> ready_nodes_mult;

    bool ready;

    while (schedule.size() < n)
    {
        for (j = 0; j < n; j++)
        {
            vector<int> row = adj_mat[j];

            ready = true; // whether all successors are scheduled

            for (i = 0; i < n; i++)
            {
                // Node i is a successor of j but has not been scheduled
                if (i != j && schedule.count(i) == 0 && row[i] == 1)
                {
                    ready = false;
                    break;
                }
            }

            // Filter out already scheduled nodes
            if (ready && schedule.count(j) == 0)
            {
                if (adj_mat[j][j] == 0)
                    ready_nodes_add.emplace(j);
                else if (adj_mat[j][j] == 1)
                    ready_nodes_sub.emplace(j);
                else if (adj_mat[j][j] == 2)
                    ready_nodes_mult.emplace(j);
            }
        }

        vector<int> next_add =
            get_top(slack, constraint[0], ready_nodes_add);

        vector<int> next_sub =
            get_top(slack, constraint[1], ready_nodes_sub);

        vector<int> next_mult =
            get_top(slack, constraint[2], ready_nodes_mult);

        if (next_add.empty() && next_sub.empty() && next_mult.empty())
        {
            cout << "cannot schedule" << endl;
            break;
        }

        /// cout << "ready additions: ";

        for (const auto &node : next_add)
        {
            // cout << node << " ";
            ready_nodes_add.erase(node);
            schedule[node] = cycle;
        }
        // cout << endl;

        // cout << "ready sub: ";

        for (const auto &node : next_sub)
        {
            // cout << node << " ";
            ready_nodes_sub.erase(node);
            schedule[node] = cycle;
        }
        // cout << endl;

        // cout << "ready mult: ";

        for (const auto &node : next_mult)
        {
            // cout << node << " ";
            ready_nodes_mult.erase(node);
            schedule[node] = cycle;
        }
        // cout << endl;

        cycle--;
    }

    // adjust to starting at cycle 0
    for (i = 0; i < n; i++)
    {
        schedule[i] -= (cycle + 1);
    }
    total_cycle = 19 - cycle;

    return schedule;
}

// finds the interval for which a node is allowed to be schedules
pair<int, int> calc_interval(vector<vector<int>> adj_mat, int node, map<int, int> &schedule, map<int, int> asap, map<int, int> alap)
{
    int first = asap[node];
    int last = alap[node];
    size_t n = adj_mat.size();
    for (size_t predecessor = 0; predecessor < n; predecessor++)
    {
        if (adj_mat[predecessor][node] == 1 &&
            schedule.count(predecessor) != 0)
        {
            first = max(first, schedule[predecessor] + 1);
        }
    }
    for (size_t successor = 0; successor < n; successor++)
    {
        if (adj_mat[node][successor] == 1 &&
            schedule.count(successor) != 0)
        {
            last = min(last, schedule[successor] - 1);
        }
    }
    return pair<int, int>(first, last);
}

// calculated the cost of a given schedule, cost reflects how spread out resource utilization is
double calc_cost(vector<vector<int>> adj_mat, map<int, int> schedule, int total_cycle, map<int, int> asap, map<int, int> alap)
{
    vector<vector<double>> utilization(3, vector<double>(total_cycle, 0.0));

    size_t n = adj_mat.size();
    for (size_t node = 0; node < n; node++)
    {
        int resource = adj_mat[node][node];

        // utilization is 1 for already scheduled nodes
        if (schedule.count(node) != 0)
        {
            int cycle = schedule[node];
            utilization[resource][cycle] += 1.0;
        }
        // utilization is 1/interval_length for uncertain nodes
        else
        {
            pair<int, int> bounds = calc_interval(adj_mat, node, schedule, asap, alap);

            double probability = 1.0 / (bounds.second - bounds.first + 1);
            for (int cycle = bounds.first; cycle <= bounds.second; ++cycle)
            {
                utilization[resource][cycle] += probability;
            }
        }
    }

    double cost = 0.0;
    for (const auto &resource_utilization : utilization)
    {
        for (double value : resource_utilization)
        {
            cost += value * value;
        }
    }
    return cost;
}

// force directed scheduling: balance resource utilization
map<int, int> FDS(vector<vector<int>> adj_mat, int &total_cycle, map<int, int> asap, map<int, int> alap)
{
    size_t n = adj_mat.size();
    size_t i, j;

    // start with alap and move things around
    int last_cycle = 0;
    for (i = 0; i < n; i++)
    {
        last_cycle = max(last_cycle, alap[i]);
    }
    total_cycle = last_cycle + 1;

    map<int, int> schedule;

    while (schedule.size() < n)
    {
        double current_cost = calc_cost(adj_mat, schedule, total_cycle, asap, alap);
        int selected_node = -1;
        int selected_cycle = -1;
        double selected_cost = numeric_limits<double>::infinity();

        for (size_t node = 0; node < n; ++node)
        {
            // skip already scheduled
            if (schedule.count(node) != 0)
            {
                continue;
            }

            pair<int, int> bounds = calc_interval(adj_mat, node, schedule, asap, alap);
            if (bounds.first > bounds.second)
            {
                cout << "invalid interval" << endl;
                return schedule;
            }
            for (int cycle = bounds.first; cycle <= bounds.second; ++cycle)
            {
                map<int, int> candidate = schedule;
                candidate[node] = cycle;
                // check if changed beats previous schedule
                double force = calc_cost(adj_mat, candidate, total_cycle, asap, alap) - current_cost;
                if (force < selected_cost)
                {
                    selected_cost = force;
                    selected_node = node;
                    selected_cycle = cycle;
                }
            }
        }

        if (selected_node == -1)
        {
            cout << "cannot schedule" << endl;
            return schedule;
        }
        schedule[selected_node] = selected_cycle;
    }

    return schedule;
}
