#pragma once
#include <vector>
#include <string>

struct Process {
    int pid{};
    int arrival_time{};
    int burst_time{};
    int remaining_time{};     // for Round Robin
    int priority{};           // lower number = higher priority (we'll follow this convention)

    std::vector<int> max_need; // requested resources (R1..Rm) for this process

    // Stats
    int start_time{-1};
    int finish_time{-1};

    Process() = default;

    Process(int id, int at, int bt, int pr, std::vector<int> need)
        : pid(id), arrival_time(at), burst_time(bt), remaining_time(bt),
          priority(pr), max_need(std::move(need)) {}
};
