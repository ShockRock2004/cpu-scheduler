#pragma once
#include <vector>

#include "scheduler.hpp"

struct Stats {
    double avgTurnaround, avgWait, avgResponse, throughput;
    int maxWait, switches, makespan;
};

Stats simulate(Scheduler& s, std::vector<Process> procs, int switchCost);
