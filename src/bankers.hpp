#pragma once
#include <vector>
#include <unordered_map>
#include <optional>
#include "process.hpp"

class Bankers {
public:
    explicit Bankers(std::vector<int> available);

    // Try to grant "request" to pid. If safe => grant + return safe sequence.
    // If unsafe => do NOT change state and return nullopt.
    std::optional<std::vector<int>> request_resources(int pid,
                                                      const std::vector<int>& max_claim);

    // Release all resources of pid (when process finishes)
    void release_all(int pid);

    const std::vector<int>& available() const { return available_; }

private:
    std::vector<int> available_;
    // allocation[pid] = currently allocated resources to pid
    std::unordered_map<int, std::vector<int>> allocation_;
    // max_need[pid] = maximum claim for pid
    std::unordered_map<int, std::vector<int>> max_need_;

    bool safety_check(std::vector<int>& out_finish_order) const;

    static bool leq(const std::vector<int>& a, const std::vector<int>& b);
    static std::vector<int> add(const std::vector<int>& a, const std::vector<int>& b);
    static std::vector<int> sub(const std::vector<int>& a, const std::vector<int>& b);
};
