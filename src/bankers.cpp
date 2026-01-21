#include "bankers.hpp"
#include <stdexcept>

Bankers::Bankers(std::vector<int> available)
    : available_(std::move(available)) {
    if (available_.empty()) throw std::invalid_argument("available vector must not be empty");
}

bool Bankers::leq(const std::vector<int>& a, const std::vector<int>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) if (a[i] > b[i]) return false;
    return true;
}

std::vector<int> Bankers::add(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> r(a.size(), 0);
    for (size_t i = 0; i < a.size(); i++) r[i] = a[i] + b[i];
    return r;
}

std::vector<int> Bankers::sub(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> r(a.size(), 0);
    for (size_t i = 0; i < a.size(); i++) r[i] = a[i] - b[i];
    return r;
}

std::optional<std::vector<int>> Bankers::request_resources(int pid,
                                                           const std::vector<int>& max_claim)
{
    // Save max claim
    max_need_[pid] = max_claim;

    // current allocation (default 0)
    if (!allocation_.count(pid)) allocation_[pid] = std::vector<int>(available_.size(), 0);

    // In our lab design: when admitted to run, it requests its FULL max claim at once.
    // (Later you can extend to partial requests.)
    const auto& alloc = allocation_.at(pid);
    auto need = sub(max_need_.at(pid), alloc);

    // If need > available => cannot grant immediately
    if (!leq(need, available_)) return std::nullopt;

    // Tentatively allocate
    auto old_avail = available_;
    auto old_alloc = allocation_[pid];

    available_ = sub(available_, need);
    allocation_[pid] = add(allocation_[pid], need);

    std::vector<int> safe_seq;
    if (!safety_check(safe_seq)) {
        // rollback
        available_ = old_avail;
        allocation_[pid] = old_alloc;
        return std::nullopt;
    }

    return safe_seq;
}

void Bankers::release_all(int pid) {
    auto it = allocation_.find(pid);
    if (it == allocation_.end()) return;
    available_ = add(available_, it->second);
    allocation_.erase(pid);
    max_need_.erase(pid);
}

bool Bankers::safety_check(std::vector<int>& out_finish_order) const {
    out_finish_order.clear();
    std::vector<int> work = available_;

    // finish flags for all processes in allocation_
    std::unordered_map<int, bool> finish;
    for (const auto& kv : allocation_) finish[kv.first] = false;

    bool progress = true;
    while (progress) {
        progress = false;

        for (const auto& kv : allocation_) {
            int pid = kv.first;
            if (finish[pid]) continue;

            auto alloc = kv.second;
            auto maxc = max_need_.at(pid);
            auto need = sub(maxc, alloc);

            if (leq(need, work)) {
                // pretend pid finishes and releases
                work = add(work, alloc);
                finish[pid] = true;
                out_finish_order.push_back(pid);
                progress = true;
            }
        }
    }

    // If any process not finished => unsafe
    for (const auto& kv : finish) {
        if (!kv.second) return false;
    }
    return true;
}
