#pragma once
#include <vector>
#include <unordered_map>
#include "process.hpp"

struct GanttSlice {
    int pid;
    int start;
    int end;
};

struct ScheduleResult {
    std::vector<GanttSlice> gantt;
    std::unordered_map<int,int> waiting;
    std::unordered_map<int,int> turnaround;
    double avg_waiting{0};
    double avg_turnaround{0};
    int finish_time{0};
};

class Scheduler {
public:
    // Rule: <=5 Priority, >5 Round Robin (q=4)
    static ScheduleResult run(std::vector<Process> procs, int quantum=4);

private:
    static ScheduleResult priority_nonpreemptive(std::vector<Process> procs);
    static ScheduleResult round_robin(std::vector<Process> procs, int quantum);

    static void compute_stats(std::vector<Process>& done, ScheduleResult& out);
};
