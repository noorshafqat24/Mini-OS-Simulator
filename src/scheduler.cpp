#include "scheduler.hpp"
#include <algorithm>
#include <queue>

ScheduleResult Scheduler::run(std::vector<Process> procs, int quantum) {
    if (procs.size() <= 5) return priority_nonpreemptive(std::move(procs));
    return round_robin(std::move(procs), quantum);
}

// Assumption: Priority scheduling = NON-preemptive (common in labs).
// If your teacher wants preemptive priority, we can modify later.
ScheduleResult Scheduler::priority_nonpreemptive(std::vector<Process> procs) {
    ScheduleResult out;

    std::sort(procs.begin(), procs.end(), [](const Process& a, const Process& b){
        if (a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
        return a.priority < b.priority; // lower = higher priority
    });

    int t = 0;
    std::vector<Process> done;
    std::vector<Process> ready;
    size_t i = 0;

    while (i < procs.size() || !ready.empty()) {
        while (i < procs.size() && procs[i].arrival_time <= t) {
            ready.push_back(procs[i]);
            i++;
        }

        if (ready.empty()) {
            t = procs[i].arrival_time;
            continue;
        }

        auto it = std::min_element(ready.begin(), ready.end(), [](const Process& x, const Process& y){
            if (x.priority != y.priority) return x.priority < y.priority;
            return x.arrival_time < y.arrival_time;
        });

        Process p = *it;
        ready.erase(it);

        if (p.start_time == -1) p.start_time = t;
        int start = t;
        t += p.burst_time;
        p.remaining_time = 0;
        p.finish_time = t;

        out.gantt.push_back({p.pid, start, t});
        done.push_back(p);
    }

    out.finish_time = t;
    compute_stats(done, out);
    return out;
}

ScheduleResult Scheduler::round_robin(std::vector<Process> procs, int quantum) {
    ScheduleResult out;

    std::sort(procs.begin(), procs.end(), [](const Process& a, const Process& b){
        return a.arrival_time < b.arrival_time;
    });

    std::queue<Process> q;
    std::vector<Process> done;
    size_t i = 0;
    int t = 0;

    while (i < procs.size() || !q.empty()) {
        while (i < procs.size() && procs[i].arrival_time <= t) {
            q.push(procs[i]);
            i++;
        }

        if (q.empty()) {
            t = procs[i].arrival_time;
            continue;
        }

        Process p = q.front(); q.pop();
        if (p.start_time == -1) p.start_time = t;

        int run = std::min(quantum, p.remaining_time);
        int start = t;
        t += run;
        p.remaining_time -= run;

        out.gantt.push_back({p.pid, start, t});

        while (i < procs.size() && procs[i].arrival_time <= t) {
            q.push(procs[i]);
            i++;
        }

        if (p.remaining_time > 0) {
            q.push(p);
        } else {
            p.finish_time = t;
            done.push_back(p);
        }
    }

    out.finish_time = t;
    compute_stats(done, out);
    return out;
}

void Scheduler::compute_stats(std::vector<Process>& done, ScheduleResult& out) {
    double sum_w = 0, sum_t = 0;

    for (auto& p : done) {
        int tat = p.finish_time - p.arrival_time;
        int wt  = tat - p.burst_time;

        out.turnaround[p.pid] = tat;
        out.waiting[p.pid] = wt;

        sum_w += wt;
        sum_t += tat;
    }

    if (!done.empty()) {
        out.avg_waiting = sum_w / done.size();
        out.avg_turnaround = sum_t / done.size();
    }
}
