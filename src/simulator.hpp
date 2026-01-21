#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#include "ready_buffer.hpp"
#include "bankers.hpp"
#include "scheduler.hpp"
#include "process.hpp"

class Simulator {
public:
    Simulator(int buffer_size,
              int producers_count,
              int processes_per_producer,
              std::vector<int> initial_available);

    // Runs one full simulation cycle
    void start();

    // Menu actions
    void add_process(const Process& p);
    void display_state() const;

private:
    ReadyBuffer buffer_;
    Bankers banker_;

    int producers_count_;
    int processes_per_producer_;

    std::atomic<int> next_pid_{1};

    // Lists for integration
    std::vector<Process> ready_list_;
    std::vector<Process> blocked_list_;

    // Processes added via menu before "Start Simulation"
    std::vector<Process> manual_pool_;

    // Must be mutable because display_state() is const but needs locking
    mutable std::mutex lists_mtx_;

    // Thread functions
    void producer_thread(int id);
    void consumer_dispatcher();

    // Blocked handling
    void try_unblock();
    void print_blocked() const;

    // Helpers
    static Process make_sentinel();
};

