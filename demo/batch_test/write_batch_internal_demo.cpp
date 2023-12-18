//
// Created by wangyz38535 on 2023/12/5.
//

#include "write_batch_internal_demo.h"

using namespace WRITE_BATCH_DEMO;

uint32_t WriteBatchInternal::Count(const WriteBatch *batch) {
    //   8-byte sequence number
    const auto *const buffer =  reinterpret_cast<const uint8_t *>(batch->rep_.data() + 8);

    // Recent clang and gcc optimize this to a single mov / ldr instruction.
    return (static_cast<uint32_t>(buffer[0])) |
            (static_cast<uint32_t>(buffer[1]) << 8) |
            (static_cast<uint32_t>(buffer[2]) << 16) |
            (static_cast<uint32_t>(buffer[3]) << 24);
}
