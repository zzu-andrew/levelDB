// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_UTIL_ARENA_H_
#define STORAGE_LEVELDB_UTIL_ARENA_H_

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace leveldb {

    class Arena {
    public:
        Arena();

        Arena(const Arena &) = delete;

        Arena &operator=(const Arena &) = delete;

        ~Arena();

        // Return a pointer to a newly allocated memory block of "bytes" bytes.
        char *Allocate(size_t bytes);

        // Allocate memory with the normal alignment guarantees provided by malloc.
        char *AllocateAligned(size_t bytes);

        // Returns an estimate of the total memory usage of data allocated
        // by the arena.
        size_t MemoryUsage() const {
            return memory_usage_.load(std::memory_order_relaxed);
        }

    private:
        char *AllocateFallback(size_t bytes);

        char *AllocateNewBlock(size_t block_bytes);

        // Allocation state
        char *alloc_ptr_;
        size_t alloc_bytes_remaining_;

        // Array of new[] allocated memory blocks
        std::vector<char *> blocks_;

        // Total memory usage of the arena.
        //
        // TODO(costan): This member is accessed via atomics, but the others are
        //               accessed without any locking. Is this OK?
        std::atomic<size_t> memory_usage_;
    };

    inline char *Arena::Allocate(size_t bytes) {
        // The semantics of what to return are a bit messy if we allow
        // 0-byte allocations, so we disallow them here (we don't need
        // them for our internal use).
        // 如果允许申请0字节的内存，会造成很多换乱的问题，因此内部接口中我们禁止这种使用方式，bytes要确保大于0
        assert(bytes > 0);
        // 如果上次申请的内存还够用
        if (bytes <= alloc_bytes_remaining_) {
            // 记录当前指针地址
            char *result = alloc_ptr_;
            // 将当前指针向后移动bytes
            alloc_ptr_ += bytes;
            // 剩余的内存大小在原有的基础上要减少 bytes
            alloc_bytes_remaining_ -= bytes;
            // 将地址放回
            return result;
        }
        // 如果上次申请剩余的内存不够，或者第一次进来申请内存，就新申请内存
        return AllocateFallback(bytes);
    }

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_UTIL_ARENA_H_
