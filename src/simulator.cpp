#include "simulator.hpp"
#include <iostream>
#include <chrono>
#include <algorithm> // for sort

Process Simulator::make_sentinel() {
    return Process{-1, 0, 0, 0, {}};
}

Simulator::Simulator(int buffer_size,
                     int producers_count,
                     int processes_per_producer,
                     std::vector<int> initial_available)
    : buffer_(buffer_size),
      banker_(std::move(initial_available)),
      producers_count_(producers_count),
      processes_per_producer_(processes_per_producer) {}

void Simulator::add_process(const Process& p) {
    std::lock_guard<std::mutex> lock(lists_mtx_);
    manual_pool_.push_back(p);
    std::cout << "[Menu] Added process PID=" << p.pid << "\n";
}

void Simulator::display_state() const {
    std::lock_guard<std::mutex> lock(lists_mtx_);

    std::cout << "\n--- Current State ---\n";
    std::cout << "Available: ";
    for (int x : banker_.available()) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "Manual pool: ";
    if (manual_pool_.empty()) std::cout << "(none)";
    for (auto &p : manual_pool_) std::cout << "P" << p.pid << " ";
    std::cout << "\n";

    std::cout << "Ready list: ";
    if (ready_list_.empty()) std::cout << "(none)";
    for (auto &p : ready_list_) std::cout << "P" << p.pid << " ";
    std::cout << "\n";

    std::cout << "Blocked list: ";
    if (blocked_list_.empty()) std::cout << "(none)";
    for (auto &p : blocked_list_) std::cout << "P" << p.pid << " ";
    std::cout << "\n";
    std::cout << "---------------------\n";
}

void Simulator::producer_thread(int id) {
    for (int i = 0; i < processes_per_producer_; i++) {
        int pid = next_pid_.fetch_add(1);

        int burst = (pid % 7) + 1;
        int pr = (pid % 5) + 1;

        // arrival time now matters
        int at = i;

        // Must match number of resource types (we use 3 in main)
        std::vector<int> need = { pid % 3, (pid + 1) % 3, (pid + 2) % 3 };

        Process p(pid, at, burst, pr, need);

        std::cout << "[Producer " << id << "] push PID=" << pid
                  << " at=" << at
                  << " burst=" << burst
                  << " pr=" << pr << "\n";

        buffer_.push(p);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

void Simulator::consumer_dispatcher() {
    while (true) {
        Process p = buffer_.pop();
        if (p.pid == -1) {
            std::cout << "[Consumer] got sentinel, stopping intake.\n";
            break;
        }

        auto safe = banker_.request_resources(p.pid, p.max_need);

        std::lock_guard<std::mutex> lock(lists_mtx_);
        if (safe) {
            std::cout << "[Consumer] PID=" << p.pid << " SAFE -> ready. SafeSeq: ";
            for (int x : *safe) std::cout << x << " ";
            std::cout << "\n";
            ready_list_.push_back(p);
        } else {
            std::cout << "[Consumer] PID=" << p.pid << " UNSAFE -> blocked.\n";
            blocked_list_.push_back(p);
        }
    }
}

void Simulator::try_unblock() {
    // assumes lists_mtx_ already held by caller
    std::vector<Process> still_blocked;

    for (auto &p : blocked_list_) {
        auto safe = banker_.request_resources(p.pid, p.max_need);
        if (safe) {
            std::cout << "[Unblock] PID=" << p.pid << " now SAFE. SafeSeq: ";
            for (int x : *safe) std::cout << x << " ";
            std::cout << "\n";
            ready_list_.push_back(p);
        } else {
            still_blocked.push_back(p);
        }
    }

    blocked_list_ = std::move(still_blocked);
}

void Simulator::print_blocked() const {
    std::cout << "\nBlocked processes: ";
    if (blocked_list_.empty()) {
        std::cout << "(none)\n";
        return;
    }
    for (auto &p : blocked_list_) std::cout << "P" << p.pid << " ";
    std::cout << "\n";
}

void Simulator::start() {
    // reset lists each run (important for menu-based repeated runs)
    {
        std::lock_guard<std::mutex> lock(lists_mtx_);
        ready_list_.clear();
        blocked_list_.clear();
    }

    std::cout << "\n=== Simulation Start ===\n";
    std::cout << "Initial Available Resources: ";
    for (int x : banker_.available()) std::cout << x << " ";
    std::cout << "\n";

    // Decide mode: manual or threaded
    std::vector<Process> manual_copy;
    {
        std::lock_guard<std::mutex> lock(lists_mtx_);
        manual_copy = manual_pool_;
        manual_pool_.clear();
    }

    if (!manual_copy.empty()) {
        std::cout << "[Simulation] Using manual processes (" << manual_copy.size() << ")\n";

        // Push manual processes then sentinel
        for (auto &p : manual_copy) buffer_.push(p);
        buffer_.push(make_sentinel());

        // Run dispatcher in same thread
        consumer_dispatcher();
    } else {
        // Threaded mode (2 producers + 1 consumer) requirement
        std::thread consumer(&Simulator::consumer_dispatcher, this);

        std::vector<std::thread> producers;
        for (int i = 0; i < producers_count_; i++)
            producers.emplace_back(&Simulator::producer_thread, this, i + 1);

        for (auto &t : producers) t.join();

        buffer_.push(make_sentinel());
        consumer.join();
    }

    // ---- Scheduling + release + unblock ----
    {
        std::lock_guard<std::mutex> lock(lists_mtx_);

        std::cout << "\nReady list count = " << ready_list_.size()
                  << " -> " << (ready_list_.size() <= 5 ? "Priority" : "Round Robin")
                  << "\n";

        auto result = Scheduler::run(ready_list_, 4);

        std::cout << "\nGantt Chart:\n";
        for (auto &s : result.gantt) {
            std::cout << "| P" << s.pid << " (" << s.start << "-" << s.end << ") ";
        }
        std::cout << "|\n";

        // PRINT WT/TAT IN SORTED PID ORDER (polish fix)
        std::vector<int> pids;
        pids.reserve(result.waiting.size());
        for (auto &kv : result.waiting) pids.push_back(kv.first);
        std::sort(pids.begin(), pids.end());

        std::cout << "\nPID  WT  TAT\n";
        for (int pid : pids) {
            std::cout << pid << "   " << result.waiting.at(pid)
                      << "   " << result.turnaround.at(pid) << "\n";
        }

        std::cout << "\nAverage WT=" << result.avg_waiting
                  << "  Average TAT=" << result.avg_turnaround << "\n";

        // Release resources for finished processes
        for (auto &p : ready_list_) banker_.release_all(p.pid);

        // try to unblock after resources are released
        try_unblock();
        print_blocked();
    }

    std::cout << "\n=== Simulation End ===\n";
}
