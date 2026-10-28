#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

template <typename T>
class LockFreeSpscQueue {
private:
#ifdef __cpp_lib_hardware_interference_size
    static constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
#else
    static constexpr size_t cache_line_size = 64;
#endif
    static constexpr size_t buffer_alignment = std::max(cache_line_size, alignof(T));
    static constexpr std::align_val_t buffer_align_val = static_cast<std::align_val_t>(buffer_alignment);

    size_t buffer_capacity_; // capacity + 1 for the sentinel
    T* buffer_;

    alignas(cache_line_size) std::atomic<size_t> write_index_;
    alignas(cache_line_size) std::atomic<size_t> read_index_;

public:
    explicit LockFreeSpscQueue(const size_t capacity) : buffer_capacity_(capacity + 1), write_index_(0), read_index_(0) {
        // round up alloc size to buffer alignment
        size_t alloc_size = ((buffer_capacity_ * sizeof(T)) + buffer_alignment - 1) / buffer_alignment * buffer_alignment;
        buffer_ = static_cast<T*>(::operator new(alloc_size, buffer_align_val));
    }

    ~LockFreeSpscQueue() {
        // while (front()) {
        //     pop();
        // }
        ::operator delete(buffer_, buffer_align_val);
    }

    LockFreeSpscQueue(const LockFreeSpscQueue&) = delete;
    LockFreeSpscQueue& operator=(const LockFreeSpscQueue&) = delete;
    LockFreeSpscQueue(const LockFreeSpscQueue&&) = delete;
    LockFreeSpscQueue& operator=(const LockFreeSpscQueue&&) = delete;

    template <typename... Args>
    bool try_emplace(Args&&... args) {
        const size_t write_index = write_index_.load(std::memory_order_relaxed);
        size_t next_write_index = write_index + 1;
        if (next_write_index == buffer_capacity_) {
            next_write_index = 0;
        }

        if (next_write_index == read_index_.load(std::memory_order_acquire)) {
            return false;
        }

        new (&buffer_[write_index]) T(std::forward<Args>(args)...);
        write_index_.store(next_write_index, std::memory_order_release);
        return true;
    }

    bool try_push(const T& value) { 
        return try_emplace(value); 
    }

    bool try_push(T&& value) { 
        return try_emplace(std::move(value));
    }
    
};
