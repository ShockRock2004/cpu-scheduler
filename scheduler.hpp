#pragma once
#include <algorithm>
#include <array>
#include <deque>
#include <string>

struct Process {
    int id, arrival, burst, priority;
    int remaining = 0, start = -1, finish = 0, readySince = 0, level = 0;
};

class Scheduler {
public:
    virtual ~Scheduler() = default;
    virtual std::string name() const = 0;
    virtual void add(Process* p, int now) { p->readySince = now; ready.push_back(p); }
    virtual void requeue(Process* p, int /*ranFor*/, int now) { add(p, now); }
    virtual Process* pick(int now) = 0;
    virtual bool preempt(const Process*, int /*ranFor*/, int /*now*/) { return false; }
    virtual void tick(int /*now*/) {}
    virtual bool empty() const { return ready.empty(); }

protected:
    std::deque<Process*> ready;

    Process* takeFront() { Process* p = ready.front(); ready.pop_front(); return p; }
    template <class Key> Process* takeMin(Key key) {
        auto it = std::min_element(ready.begin(), ready.end(),
                                   [&](Process* a, Process* b) { return key(a) < key(b); });
        Process* p = *it;
        ready.erase(it);
        return p;
    }
};

class FCFS : public Scheduler {
public:
    std::string name() const override { return "FCFS"; }
    Process* pick(int) override { return takeFront(); }
};

class SJF : public Scheduler {
public:
    std::string name() const override { return "SJF"; }
    Process* pick(int) override { return takeMin([](Process* p) { return p->burst; }); }
};

class SRTF : public Scheduler {
public:
    std::string name() const override { return "SRTF"; }
    Process* pick(int) override { return takeMin([](Process* p) { return p->remaining; }); }
    bool preempt(const Process* r, int, int) override {
        return std::any_of(ready.begin(), ready.end(), [&](Process* p) { return p->remaining < r->remaining; });
    }
};

class RoundRobin : public Scheduler {
public:
    explicit RoundRobin(int quantum) : q(quantum) {}
    std::string name() const override { return "RR(q=" + std::to_string(q) + ")"; }
    Process* pick(int) override { return takeFront(); }
    bool preempt(const Process*, int ranFor, int) override { return ranFor >= q && !ready.empty(); }
private:
    int q;
};

class Priority : public Scheduler {
public:
    std::string name() const override { return "Priority"; }
    Process* pick(int) override { return takeMin([](Process* p) { return p->priority; }); }
};

class HRRN : public Scheduler {
public:
    std::string name() const override { return "HRRN"; }
    Process* pick(int now) override {
        return takeMin([now](Process* p) { return -double(now - p->readySince + p->burst) / p->burst; });
    }
};

class AgingPriority : public Scheduler {
public:
    explicit AgingPriority(int interval) : every(interval) {}
    std::string name() const override { return "Priority+Aging"; }
    Process* pick(int now) override {
        return takeMin([&](Process* p) { return p->priority - (now - p->readySince) / every; });
    }
private:
    int every;
};

class MLFQ : public Scheduler {
public:
    explicit MLFQ(int agingThreshold) : aging(agingThreshold) {}
    std::string name() const override { return "MLFQ+Aging"; }

    void add(Process* p, int now) override { p->readySince = now; q[p->level].push_back(p); }

    void requeue(Process* p, int ranFor, int now) override {
        if (p->level < 2 && ranFor >= quantum[p->level]) p->level++;
        add(p, now);
    }

    Process* pick(int) override {
        for (auto& level : q)
            if (!level.empty()) { Process* p = level.front(); level.pop_front(); return p; }
        return nullptr;
    }

    bool preempt(const Process* r, int ranFor, int) override {
        for (int l = 0; l < r->level; l++)
            if (!q[l].empty()) return true;
        return r->level < 2 && ranFor >= quantum[r->level];
    }

    void tick(int now) override {
        for (int l = 1; l < 3; l++)
            for (auto it = q[l].begin(); it != q[l].end();) {
                if (now - (*it)->readySince < aging) { ++it; continue; }
                Process* p = *it;
                it = q[l].erase(it);
                p->level = 0;
                add(p, now);
            }
    }

    bool empty() const override {
        return std::all_of(q.begin(), q.end(), [](const auto& l) { return l.empty(); });
    }

private:
    std::array<std::deque<Process*>, 3> q;
    int quantum[2] = {4, 8};
    int aging;
};
