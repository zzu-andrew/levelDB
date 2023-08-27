// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "util/arena.h"

namespace leveldb {

    static const int kBlockSize = 4096;

    Arena::Arena()
            : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}

    Arena::~Arena() {
        for (auto & block : blocks_) {
            delete[] block;
        }
    }

    char *Arena::AllocateFallback(size_t bytes) {
        if (bytes > kBlockSize / 4) {
            // Object is more than a quarter of our block size.  Allocate it separately
            // to avoid wasting too much space in leftover bytes.
            // 申请的原则
            // 如果申请的内存大于指定block size的四分之一，就按照指定内存大小进行申请
            char *result = AllocateNewBlock(bytes);
            return result;
        }

        // We waste the remaining space in the current block.
        // 如果需要的大小小于1024字节，那么按照个4096字节大小申请，
        alloc_ptr_ = AllocateNewBlock(kBlockSize);
        // 申请之后将remaining大小修改为申请的大小
        alloc_bytes_remaining_ = kBlockSize;

        char *result = alloc_ptr_;
        // 指针向前移动指定字节
        alloc_ptr_ += bytes;
        // 剩余可用内存减去已经使用的内存
        alloc_bytes_remaining_ -= bytes;
        return result;
    }

    char *Arena::AllocateAligned(size_t bytes) {
        // 如果当前系统指针大于8字节，就按照指针大小进行对齐，如果不是就按照8字节对齐
        const int align = (sizeof(void *) > 8) ? sizeof(void *) : 8;
        // 确保对齐字节大小是2的次方
        static_assert((align & (align - 1)) == 0,
                      "Pointer size should be a power of 2");
        // 看当前的alloc_ptr_是否是8字节对齐
        size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
        // 如果 alloc_ptr_是8字节对齐，那么current_mod会等于0，slop也会是0,，如果slop是alloc_ptr_前进多少能够8字节对齐的位置
        size_t slop = (current_mod == 0 ? 0 : align - current_mod);
        // 实际需要自己的大小为slop和需要申请内存大小的和
        size_t needed = bytes + slop;
        char *result;
        if (needed <= alloc_bytes_remaining_) {
            result = alloc_ptr_ + slop;
            alloc_ptr_ += needed;
            alloc_bytes_remaining_ -= needed;
        } else {
            // AllocateFallback always returned aligned memory
            result = AllocateFallback(bytes);
        }
        // 确保申请内存的指针是8字节对齐
        assert((reinterpret_cast<uintptr_t>(result) & (align - 1)) == 0);
        return result;
    }

    char *Arena::AllocateNewBlock(size_t block_bytes) {
        char *result = new char[block_bytes];
        // 将每次new出来的指针方能如到blocks中
        blocks_.push_back(result);
        // 已经使用的内存，是一个指针的大小和指定内存大小的和
        memory_usage_.fetch_add(block_bytes + sizeof(char *),
                                std::memory_order_relaxed);
        return result;
    }

}  // namespace leveldb
