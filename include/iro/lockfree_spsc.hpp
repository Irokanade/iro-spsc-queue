#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace iro {

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

    alignas(cache_line_size) std::atomic<size_t> write_index_ = {0};
    size_t read_index_cached_ = 0;

    alignas(cache_line_size) std::atomic<size_t> read_index_ = {0};
    size_t write_index_cached_ = 0;    

public:
    explicit LockFreeSpscQueue(const size_t capacity) : buffer_capacity_(capacity + 1) {
        // round up alloc size to buffer alignment
        const size_t alloc_size = ((buffer_capacity_ * sizeof(T)) + buffer_alignment - 1) / buffer_alignment * buffer_alignment;
        buffer_ = static_cast<T*>(::operator new(alloc_size, buffer_align_val));
    }

    ~LockFreeSpscQueue() {
        while (front()) {
            pop();
        }
        ::operator delete(buffer_, buffer_align_val);
    }

    LockFreeSpscQueue(const LockFreeSpscQueue&) = delete;
    LockFreeSpscQueue& operator=(const LockFreeSpscQueue&) = delete;
    LockFreeSpscQueue(LockFreeSpscQueue&&) = delete;
    LockFreeSpscQueue& operator=(LockFreeSpscQueue&&) = delete;

    template <typename... Args>
        requires std::constructible_from<T, Args&&...>
    void emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) {
        const size_t write_index = write_index_.load(std::memory_order_relaxed);
        size_t next_write_index = write_index + 1;
        if (next_write_index == buffer_capacity_) {
            next_write_index = 0;
        }

        while (next_write_index == read_index_cached_) {
            read_index_cached_ = read_index_.load(std::memory_order_acquire);
        }

        std::construct_at(buffer_ + write_index, std::forward<Args>(args)...);
        write_index_.store(next_write_index, std::memory_order_release);
    }

    template <typename... Args>
        requires std::constructible_from<T, Args&&...>
    bool try_emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>) {
        const size_t write_index = write_index_.load(std::memory_order_relaxed);
        size_t next_write_index = write_index + 1;
        if (next_write_index == buffer_capacity_) {
            next_write_index = 0;
        }

        if (next_write_index == read_index_cached_) {
            read_index_cached_ = read_index_.load(std::memory_order_acquire);
            if (next_write_index == read_index_cached_) {
                return false;
            }
        }

        std::construct_at(buffer_ + write_index, std::forward<Args>(args)...);
        write_index_.store(next_write_index, std::memory_order_release);
        return true;
    }

    void push(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        emplace(value);
    }

    void push(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
        emplace(std::move(value));
    }

    bool try_push(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        return try_emplace(value);
    }

    bool try_push(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) {
        return try_emplace(std::move(value));
    }

    T* front() noexcept {
        const size_t read_index = read_index_.load(std::memory_order_relaxed);
        if (read_index == write_index_cached_) {
            write_index_cached_ = write_index_.load(std::memory_order_acquire);
            if (read_index == write_index_cached_) {
                return nullptr;
            }
        }
        return &buffer_[read_index];
    }

    void pop() noexcept {
        const size_t read_index = read_index_.load(std::memory_order_relaxed);
        buffer_[read_index].~T();

        size_t next_read_index = read_index + 1;
        if (next_read_index == buffer_capacity_) {
            next_read_index = 0;
        }
        read_index_.store(next_read_index, std::memory_order_release);
    }

    size_t size() const noexcept {
        const size_t write_index = write_index_.load(std::memory_order_acquire);
        const size_t read_index = read_index_.load(std::memory_order_acquire);
        if (write_index >= read_index) {
            return write_index - read_index;
        }
        return write_index + buffer_capacity_ - read_index;
    }

    bool empty() const noexcept {
        return write_index_.load(std::memory_order_acquire) == read_index_.load(std::memory_order_acquire);
    }

    size_t capacity() const noexcept {
        return buffer_capacity_ - 1;
    }
};

} // namespace iro