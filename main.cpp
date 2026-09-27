#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "simulator.hpp"

static std::vector<Process> load(const char* path) {
    std::vector<Process> procs;
    std::ifstream in(path);
    int arrival, burst, priority;
    while (in >> arrival >> burst >> priority) procs.push_back({int(procs.size()) + 1, arrival, burst, priority});
    return procs;
}

static std::vector<Process> generate(int n, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> gap(0, 4), burst(1, 20), prio(1, 10);
    std::vector<Process> procs;
    for (int i = 0, t = 0; i < n; i++, t += gap(rng)) procs.push_back({i + 1, t, burst(rng), prio(rng)});
    return procs;
}

int main(int argc, char** argv) {
    int switchCost = 1;
    std::vector<Process> procs;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--cs" && i + 1 < argc) switchCost = std::atoi(argv[++i]);
        else procs = load(argv[i]);
    }
    if (procs.empty()) procs = generate(30, 42);

    std::vector<std::unique_ptr<Scheduler>> algos;
    algos.emplace_back(new FCFS);
    algos.emplace_back(new SJF);
    algos.emplace_back(new SRTF);
    algos.emplace_back(new RoundRobin(4));
    algos.emplace_back(new Priority);
    algos.emplace_back(new HRRN);
    algos.emplace_back(new AgingPriority(5));
    algos.emplace_back(new MLFQ(30));

    std::printf("%zu processes, context switch cost %d\n\n", procs.size(), switchCost);
    std::printf("%-16s %9s %9s %9s %9s %11s %9s\n", "Algorithm", "Avg TAT", "Avg Wait", "Avg Resp", "Max Wait",
                "Throughput", "Switches");
    for (auto& s : algos) {
        Stats st = simulate(*s, procs, switchCost);
        std::printf("%-16s %9.2f %9.2f %9.2f %9d %11.4f %9d\n", s->name().c_str(), st.avgTurnaround, st.avgWait,
                    st.avgResponse, st.maxWait, st.throughput, st.switches);
    }
}
