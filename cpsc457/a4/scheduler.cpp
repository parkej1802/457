// this is the only file you should modify and submit for grading

#include "scheduler.h"
#include "common.h"

// this is the function you should implement
//
// runs Round-Robin scheduling simulator
// input:
//   quantum = time slice
//   max_seq_len = maximum length of the reported executing sequence
//   processes[] = list of process with populated IDs, arrivals, and bursts
// output:
//   seq[] - will contain the execution sequence but trimmed to max_seq_len size
//         - idle CPU will be denoted by -1
//         - other entries will be from processes[].id
//         - sequence will be compressed, i.e. no repeated consecutive numbers
//   processes[]
//         - adjust finish_time and start_time for each process
//         - do not adjust other fields
//

void simulate_rr(
    int64_t quantum, 
    int64_t max_seq_len,
    std::vector<Process> & processes,
    std::vector<int> & seq
) {

    seq.clear();

    int64_t curr_time = 0;
    int64_t remaining_slice = 0;
    int cpu = -1;
    size_t jobs_remaining = processes.size();
    std::vector<int> rq, jq;
    std::vector<int64_t> remaining_bursts;

    remaining_bursts.resize(processes.size());

    for (const auto & p : processes) {
        jq.push_back(p.id);
        remaining_bursts[p.id] = p.burst;
    }

    while (true) {
        if (jobs_remaining == 0) {
            break;
        }

        //add process to rq
        for (size_t i = 0; i < jq.size();) {
            if (processes[jq[i]].arrival <= curr_time) {
                rq.push_back(jq[i]);
                jq.erase(jq.begin() + i);
            } 
            else {
                ++i;
            }
        }

        //if cpu is finished or time slice is exhausted
        if (cpu != -1 && (remaining_bursts[cpu] == 0 || remaining_slice == 0)) {
            if (remaining_bursts[cpu] == 0) {
                processes[cpu].finish_time = curr_time;
                jobs_remaining--;
            } 
            else {
                rq.push_back(cpu);
            }
            cpu = -1;
        }


        //if no process then, get the process from rq
        if (cpu == -1 && !rq.empty()) {
            cpu = rq.front();
            rq.erase(rq.begin());
            remaining_slice = quantum;
            if (processes[cpu].start_time == -1) {
                processes[cpu].start_time = curr_time;
            }
        }

    

        //process the cpu or add idle time
        if (cpu != -1) {
            if (remaining_bursts[cpu] > 0) {
                remaining_bursts[cpu]--;
                remaining_slice--;
            }
            if (seq.empty() || seq.back() != cpu) {
                if (seq.size() < static_cast<size_t>(max_seq_len)) {
                    seq.push_back(cpu);
                }
            }
            curr_time++;
            
        } 
        else {
            if (seq.empty() || seq.back() != -1) {
                if (seq.size() < static_cast<size_t>(max_seq_len)) {
                    seq.push_back(-1);
                }
            }
            if (!jq.empty()) {
                curr_time = std::max(curr_time, processes[jq.front()].arrival);
            }
            else {
                curr_time++;
            }
        }
    
    }
    if (!seq.empty() && seq.back() == -1) {
        seq.pop_back();
    }
}