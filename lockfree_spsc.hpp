#pragma once

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>

template <typename T>
class LockFreeSpscQueue {
private:
#ifdef __cpp_lib_hardware_interference_size
    static constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
#else
    static constexpr size_t cache_line_size = 64;
#endif

    size_t buffer_capacity; // capacity + 1 for the sentinel
    T* buffer;

    alignas(cache_line_size) std::atomic<size_t> write_index;
    alignas(cache_line_size) std::atomic<size_t> read_index;

public:
    explicit LockFreeSpscQueue(const size_t capacity) : buffer_capacity(capacity + 1), write_index(0), read_index(0) {
        // alloc size must be multiple of alignment
        size_t alloc_size = ((buffer_capacity * sizeof(T)) + cache_line_size - 1) / cache_line_size * cache_line_size;
        buffer = static_cast<T*>(std::aligned_alloc(cache_line_size, alloc_size));
        if (!buffer) {
            throw std::bad_alloc();
        }
    }

    ~LockFreeSpscQueue() { 
        std::free(buffer); 
    }

    LockFreeSpscQueue(const LockFreeSpscQueue&) = delete;
    LockFreeSpscQueue& operator=(const LockFreeSpscQueue&) = delete;
    LockFreeSpscQueue(const LockFreeSpscQueue&&) = delete;
    LockFreeSpscQueue& operator=(const LockFreeSpscQueue&&) = delete;
};
