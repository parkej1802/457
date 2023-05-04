// -------------------------------------------------------------------------------------
// this is the only file you need to edit
// -------------------------------------------------------------------------------------
//
// (c) 2023, Pavol Federl, pfederl@ucalgary.ca
// Do not distribute this file.

#include "fatsim.h"
#include <iostream>
#include <stack>
#include <algorithm>

std::vector<long> fat_check(const std::vector<long> & fat)
{
    std::vector<std::vector<long>> adj_list;
    long n = fat.size();
    std::vector<long> result;
    adj_list.resize(n);
    std::vector<long> inode;

    //create adj list of fat graph
    for(long i = 0; i < n; i++) {
        if(fat[i] != -1) {
            adj_list[fat[i]].push_back(i);
        }
        else {
            //gives me node that is connected to -1
            inode.push_back(i); 
        }
    }

    // implementing dfs in graph
    std::vector<long> longest_nodes;
    std::stack<std::pair<long, long>> stack; 
    std::vector<bool> visited(n, false);
    long max_depth = -1;
    
    //connected to -1 node will be starting node, start counting from it
    for (unsigned long i = 0; i < inode.size(); i++) {
        long start_node = inode[i];
        stack.emplace(start_node, 1); 
        visited[start_node] = true; //reduce repeating

        //now find longest chain node from starting node
        while (!stack.empty()) {
            std::pair<long, long> current = stack.top();
            stack.pop();
            long current_node = current.first;
            long depth = current.second;

            //updating max depth and longest chain nodes
            if (depth > max_depth && current_node != -1) { 
                max_depth = depth;
                longest_nodes.clear();
                longest_nodes.push_back(current_node);
            }

            // if it has same depth of other chain, add node
            else if (depth == max_depth && current_node != -1) {
                longest_nodes.push_back(current_node);
            }

            //preventing visiting mutiple times
            for (unsigned long j = 0; j < adj_list[current_node].size(); j++) {
                long next_node = adj_list[current_node][j];
                if (!visited[next_node]) {
                    visited[next_node] = true;
                    stack.emplace(next_node, depth+1);
                }
            }
        }
    }

    result = longest_nodes;
    std::sort(result.begin(), result.end());
    return result;
}