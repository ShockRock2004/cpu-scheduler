#include "simulator.hpp"

#include <algorithm>

Stats simulate(Scheduler& s, std::vector<Process> procs, int switchCost) {
    std::sort(procs.begin(), procs.end(), [](const Process& a, const Process& b) { return a.arrival < b.arrival; });
    for (auto& p : procs) p.remaining = p.burst;

    const int n = int(procs.size());
    int now = 0, next = 0, done = 0, switches = 0, ranFor = 0;
    Process *run = nullptr, *last = nullptr;

    while (done < n) {
        while (next < n && procs[next].arrival <= now) s.add(&procs[next++], now);
        s.tick(now);

        if (run && s.preempt(run, ranFor, now)) {
            s.requeue(run, ranFor, now);
            run = nullptr;
        }
        if (!run && !s.empty()) {
            run = s.pick(now);
            ranFor = 0;
            if (last && last != run) { switches++; now += switchCost; }
            if (run->start < 0) run->start = now;
            last = run;
        }
        if (!run) { now = procs[next].arrival; continue; }

        run->remaining--;
        ranFor++;
        now++;
        if (!run->remaining) { run->finish = now; done++; run = nullptr; }
    }

    Stats st{0, 0, 0, 0, 0, switches, now};
    for (auto& p : procs) {
        int tat = p.finish - p.arrival, wait = tat - p.burst;
        st.avgTurnaround += tat;
        st.avgWait += wait;
        st.avgResponse += p.start - p.arrival;
        st.maxWait = std::max(st.maxWait, wait);
    }
    st.avgTurnaround /= n;
    st.avgWait /= n;
    st.avgResponse /= n;
    st.throughput = double(n) / now;
    return st;
}
