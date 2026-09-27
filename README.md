# cpu-scheduler

A CPU scheduling simulator in C++17 that compares eight algorithms on the same workload.

| Algorithm | Preemptive |
|---|---|
| FCFS | no |
| SJF | no |
| SRTF | yes |
| Round Robin (q=4) | yes |
| Priority | no |
| HRRN | no |
| Priority + Aging | no |
| MLFQ + Aging (3 levels, q=4/8/FCFS) | yes |

Each algorithm implements the same `Scheduler` interface. The simulator charges a configurable
context-switch cost and reports the average turnaround, wait and response times, the maximum wait
(a starvation indicator), throughput, and the switch count.

MLFQ moves a process down a level when it uses up its quantum. An aging pass moves any process
that has waited too long in a lower queue back to the top level, so long jobs don't starve.

## Build & run

```sh
make
./sched                     # 30 random processes
./sched workload.txt --cs 2 # custom workload, context switch cost 2
```

Workload format: one process per line, `arrival burst priority` (a lower number means higher priority).
