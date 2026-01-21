#pragma once
#include <vector>
#include <mutex>
#include <semaphore.h>
#include "process.hpp"

// Bounded buffer for Process objects (ready queue)
class ReadyBuffer {
public:
    explicit ReadyBuffer(size_t capacity);
    ~ReadyBuffer();

    // Producer puts a process into buffer
    void push(const Process& p);

    // Consumer takes a process from buffer
    Process pop();

    size_t capacity() const { return cap_; }
     size_t size();
private:
    size_t cap_;
    std::vector<Process> buf_;
    size_t head_{0}; // pop index
    size_t tail_{0}; // push index
    size_t count_{0}; 
    sem_t empty_;    // counts empty slots
    sem_t full_;     // counts filled slots
    std::mutex mtx_; // protects buffer indices and data
};

