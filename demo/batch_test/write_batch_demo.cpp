//
// Created by wangyz38535 on 2023/12/5.
//

#include "write_batch_demo.h"
#include "write_batch_internal_demo.h"

namespace WRITE_BATCH_DEMO {

    // 最高位为1一直到非1停止就是长度的编码
    // 能这样编码是因为Key值都是字符串，能保证不会大于128
    // 这里将低位放到前面是有特殊用以的，当长度大于128时，我们只需要按照最高位是否是128进行查找
    // 当找到最高位为非128时，说明找到了所有的长度字段，这时包含最后一个非128开头的字符在内所有的字符就是这次的长度字段
    // 这种歌编码方式用在网络发送，数据序列化中非常有用
    // 可以说是序列化数组中同时存储长度和其他字符的完美解决方案
    char *EncodeVarint32(char *dst, uint32_t v) {
        // Operate on characters as unsigneds
        uint8_t *ptr = reinterpret_cast<uint8_t *>(dst);
        static const int B = 128;
        if (v < (1 << 7)) {
            *(ptr++) = v;
        } else if (v < (1 << 14)) {
            *(ptr++) = v | B;
            *(ptr++) = v >> 7;
        } else if (v < (1 << 21)) {
            *(ptr++) = v | B;
            *(ptr++) = (v >> 7) | B;
            *(ptr++) = v >> 14;
        } else if (v < (1 << 28)) {
            *(ptr++) = v | B;
            *(ptr++) = (v >> 7) | B;
            *(ptr++) = (v >> 14) | B;
            *(ptr++) = v >> 21;
        } else {
            *(ptr++) = v | B;
            *(ptr++) = (v >> 7) | B;
            *(ptr++) = (v >> 14) | B;
            *(ptr++) = (v >> 21) | B;
            *(ptr++) = v >> 28;
        }
        return reinterpret_cast<char *>(ptr);
    }

    void PutVarint32(std::string *dst, uint32_t v) {
        char buf[5];
        char *ptr = EncodeVarint32(buf, v);
        dst->append(buf, ptr - buf);
    }

    void PutLengthPrefixedSlice(std::string *dst, const leveldb::Slice &value) {
        PutVarint32(dst, value.size());
        dst->append(value.data(), value.size());
    }

    // 解码
    const char *GetVarint32PtrFallback(const char *p, const char *limit,
                                       uint32_t *value) {
        uint32_t result = 0;
        for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
            uint32_t byte = *(reinterpret_cast<const uint8_t *>(p));
            p++;
            if (byte & 128) {
                // More bytes are present
                result |= ((byte & 127) << shift);
            } else {
                result |= (byte << shift);
                *value = result;
                return reinterpret_cast<const char *>(p);
            }
        }
        return nullptr;
    }

    inline const char *GetVarint32Ptr(const char *p, const char *limit,
                                      uint32_t *value) {
        if (p < limit) {
            uint32_t result = *(reinterpret_cast<const uint8_t *>(p));
            if ((result & 128) == 0) {
                *value = result;
                return p + 1;
            }
        }
        return GetVarint32PtrFallback(p, limit, value);
    }

    bool GetVarint32(leveldb::Slice *input, uint32_t *value) {
        const char *p = input->data();
        const char *limit = p + input->size();
        const char *q = GetVarint32Ptr(p, limit, value);
        if (q == nullptr) {
            return false;
        } else {
            *input = leveldb::Slice(q, limit - q);
            return true;
        }
    }

    bool GetLengthPrefixedSlice(leveldb::Slice *input, leveldb::Slice *result) {
        uint32_t len;
        if (WRITE_BATCH_DEMO::GetVarint32(input, &len) && input->size() >= len) {
            *result = leveldb::Slice(input->data(), len);
            input->remove_prefix(len);
            return true;
        } else {
            return false;
        }
    }


    WriteBatch::WriteBatch() {
        //Clear();
    }

    WriteBatch::~WriteBatch() = default;

    void WriteBatch::Put(const leveldb::Slice &key, const leveldb::Slice &value) {
        // 每次put， batch的计数加1
        WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
        // kTypeValue 是put Deletion 是删除
        rep_.push_back(static_cast<char>(leveldb::kTypeValue));
        // 找到所有
        WRITE_BATCH_DEMO::PutLengthPrefixedSlice(&rep_, key);
        WRITE_BATCH_DEMO::PutLengthPrefixedSlice(&rep_, value);
    }

    void WriteBatch::Delete(const leveldb::Slice &key) {
        WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
        rep_.push_back(static_cast<char>(leveldb::kTypeDeletion));
        WRITE_BATCH_DEMO::PutLengthPrefixedSlice(&rep_, key);
    }

    void WriteBatch::Clear() {
        // 清空对象
        rep_.clear();
        // 预先设置为指定的头长度
        rep_.resize(kHeader);
    }

    size_t WriteBatch::ApproximateSize() const {
        return rep_.size();
    }

    void WriteBatch::Append(const WriteBatch &source) {
        WriteBatchInternal::Append(this, &source);
    }

    // 通过，Handler将具体业务和实现分离开来，这样WriteBatch就不需要关心具体业务的东西
    // 只需要处理序列化的Put和Get即可，具体的功能交给Handler实现
    leveldb::Status WriteBatch::Iterate(Handler *handler) const {
        // 1. 将rep_ 转化为 Slice切片以备后面使用
        leveldb::Slice input(rep_);

        if (input.size() < kHeader) {
            return leveldb::Status::Corruption("malformed WriteBatch (too small)");
        }

        // 2. 剔除序列化相关的无用数据
        leveldb::Slice key, value;
        int32_t found = 0;

        // 3. 循环处理所有rep_中的事务(key value值)
        while (!input.empty()) {
            // 执行次数增加
            found ++;
            // 取出tag，用于判断接下来需要处理数据的类型
            char tag = input[0];
            input.remove_prefix(1);

            switch (tag) {
                case leveldb::kTypeValue:
                    if (WRITE_BATCH_DEMO::GetLengthPrefixedSlice(&input, &key) &&
                            WRITE_BATCH_DEMO::GetLengthPrefixedSlice(&input, &value)) {
                        handler->Put(key, value);
                    } else {
                        return leveldb::Status::Corruption("Bad WriteBatch Put operation");
                    }
                    break;
                case leveldb::kTypeDeletion:
                    if (WRITE_BATCH_DEMO::GetLengthPrefixedSlice(&input, &key)) {
                        handler->Delete(key);
                    } else {
                        return leveldb::Status::Corruption("bad WriteBatch Delete operation");
                    }
                    break;
                default:
                    return leveldb::Status::Corruption("unknown WriteBatch tag");

            }
        }

        if (found != WriteBatchInternal::Count(this)) {
            return leveldb::Status::Corruption("WriteBatch has wrong count");
        } else {
            return leveldb::Status::OK();
        }
    }

}