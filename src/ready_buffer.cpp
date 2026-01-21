#include "ready_buffer.hpp"
#include <stdexcept>

ReadyBuffer::ReadyBuffer(size_t capacity)
    : cap_(capacity), buf_(capacity)
{
    if (cap_ == 0) throw std::invalid_argument("capacity must be > 0");

    // empty starts with capacity, full starts with 0
    if (sem_init(&empty_, 0, static_cast<unsigned int>(cap_)) != 0)
        throw std::runtime_error("sem_init(empty) failed");
    if (sem_init(&full_, 0, 0) != 0)
        throw std::runtime_error("sem_init(full) failed");
}

ReadyBuffer::~ReadyBuffer() {
    sem_destroy(&empty_);
    sem_destroy(&full_);
}

void ReadyBuffer::push(const Process& p) {
    sem_wait(&empty_);
    {
        std::lock_guard<std::mutex> lock(mtx_);
        buf_[tail_] = p;
        tail_ = (tail_ + 1) % cap_;
        count_++; // NEW
    }
    sem_post(&full_);
}

Process ReadyBuffer::pop() {
    sem_wait(&full_);
    Process item;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        item = buf_[head_];
        head_ = (head_ + 1) % cap_;
        count_--; // NEW
    }
    sem_post(&empty_);
    return item;
}

size_t ReadyBuffer::size() {
    std::lock_guard<std::mutex> lock(mtx_);
    return count_;
}

