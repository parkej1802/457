// this is the ONLY file you should edit and submit to D2L

#include "find_deadlock.h"
#include "common.h"

/// this is the function you need to (re)implement
///
/// parameter edges[] contains a list of request- and assignment- edges
///   example of a request edge, process "p1" resource "r1"
///     "p1 -> r1"
///   example of an assignment edge, process "XYz" resource "XYz"
///     "XYz <- XYz"
///
/// You need to process edges[] one edge at a time, and run a deadlock
/// detection after each edge. As soon as you detect a deadlock, your function
/// needs to stop processing edges and return an instance of Result structure
/// with 'index' set to the index that caused the deadlock, and 'procs' set
/// to contain names of processes that are in the deadlock.
///
/// To indicate no deadlock was detected after processing all edges, you must
/// return Result with index=-1 and empty procs.
///

class FastGraph {
    std::vector<std::vector<int>> adj_list;
    std::vector<int> out_counts;
    Word2Int w2i;
    std::vector<std::string> i2w;

public:
    FastGraph(int nodes) {
        adj_list.resize(nodes);
        out_counts.resize(nodes, 0);
        i2w.resize(nodes);
    }

    // use . to distinguish between a process and resources
    void add_edge(const std::vector<std::string> &edge) {
        int process_id = w2i.get(edge[0] + ".");
        int resource_id = w2i.get(edge[2]);
        i2w[process_id] = edge[0] + ".";
        i2w[resource_id] = edge[2];
        //if there is -> between process and resource, push the value to adj_list and increase the out count
        if (edge[1] == "->") {
            adj_list[resource_id].push_back(process_id);
            out_counts[process_id]++;

        } 
        
        //else if <- 
        else {
            adj_list[process_id].push_back(resource_id);
            out_counts[resource_id]++;
        }
    }

    std::vector<std::string> topological_sort() {
        //out = out_counts # copy out_counts so that we can modify it
        std::vector<int> out = out_counts;

        //zeros[] = find all nodes in graph with outdegree == 0
        std::vector<int> zeros;
        for (size_t i = 0; i < out.size(); i++) {
            if (out[i] == 0) {
                zeros.push_back(i);
            }
        }
        
        //while zeros is not empty
        while (!zeros.empty()) {
            //n = remove one entry from zeros[]
            int n = zeros.back();
            zeros.pop_back();
            //for every n2 in adj_list[n]:
            for (int n2 : adj_list[n]) {
                //out[n2] --
                out[n2]--;
                //if out[n2] == 0
                if (out[n2] == 0) {
                    //append n2 to zeros[]
                    zeros.push_back(n2);
                }
            }
        }
        //store names and remove dot at the end of name and returns name
        std::vector<std::string> process;
        for (size_t i = 0; i < out.size(); i++) {
            if (out[i] > 0 && i2w[i].back() == '.') {
                std::string p_name = i2w[i];
                p_name.pop_back(); 
                process.push_back(p_name);
            }
        }
        return process;
    }
};

Result find_deadlock(const std::vector<std::string> & edges)
{
    //initialize size of graph
    Result result;
    FastGraph graph(2 * edges.size());

    std::vector<std::string> toposort_value;

    for (unsigned int i = 0; i < edges.size(); i++) {
        //add edges
        graph.add_edge(split(edges[i]));
        //use topological sort the graph
        toposort_value = graph.topological_sort();
        //check size 0
        bool failed = toposort_value.size();
        //if the size is greater than 0, then it has deadlock
        if (failed) {
            //push values
            result.procs = toposort_value;
            result.index = i;
            return result;
        }
    }
    //otherwise
    result.index = -1;
    return result;
}