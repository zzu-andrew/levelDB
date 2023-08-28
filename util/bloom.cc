// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "leveldb/filter_policy.h"

#include "leveldb/slice.h"
#include "util/hash.h"

// 详解文章 https://blog.csdn.net/carbon06/article/details/80118954
namespace leveldb {

    namespace {
        uint32_t BloomHash(const Slice &key) {
            return Hash(key.data(), key.size(), 0xbc9f1d34);
        }

        class BloomFilterPolicy : public FilterPolicy {
        public:
            explicit BloomFilterPolicy(int bits_per_key) : bits_per_key_(bits_per_key) {
                // We intentionally round down to reduce probing cost a little bit
                // 按照计算公式 k = m/n * (ln2)来计算hash函数个数， m bit个数，n插入元素个数
                // leveldb 中简单处理了，一般这里会直接传入 bits_per_key 为 10进行计算
                k_ = static_cast<size_t>(bits_per_key * 0.69);  // 0.69 =~ ln(2)
                // 保证K_是处于  [1,30] 之间
                if (k_ < 1) k_ = 1;
                if (k_ > 30) k_ = 30;
            }

            const char *Name() const override { return "leveldb.BuiltinBloomFilter2"; }

            // 将传入的n个 key 存储到bloomfilter 中，bloomfilter结果使用string存储。
            void CreateFilter(const Slice *keys, int n, std::string *dst) const override {
                // Compute bloom filter size (in both bits and bytes)
                // bloomfilter需要多少bit  bits_per_key_ = (m/n) m bit个数，n插入元素个数
                size_t bits = n * bits_per_key_;

                // For small n, we can see a very high false positive rate.  Fix it
                // by enforcing a minimum bloom filter length.
                // 当bits太小的时候，会导致过滤器一直虚报错误，这里保证bits不小于64就可以了
                if (bits < 64) bits = 64;

                // 这里只是保证bits能够按照8位对齐
                size_t bytes = (bits + 7) / 8;
                bits = bytes * 8;
                // 在string中分配空间
                const size_t init_size = dst->size();
                dst->resize(init_size + bytes, 0);
                // string最后一个元素存储使用的 hash函数的个数
                dst->push_back(static_cast<char>(k_));  // Remember # of probes in filter
                // 获取string 内部的char型数组，数组指向多申请出来的内存，多申请出来的内存，用来存放Bloom hash映射值
                char *array = &(*dst)[init_size];
                for (int i = 0; i < n; i++) {
                    // 通过一次hash计算 一次delta计算，循环K_次将对应的 key在bits上进行置位
                    // Use double-hashing to generate a sequence of hash values.
                    // See analysis in [Kirsch,Mitzenmacher 2006].
                    // leveldb使用一个hash函数，每次对hash值向右移位17bit来模拟实现多个hash函数
                    uint32_t h = BloomHash(keys[i]);
                    const uint32_t delta = (h >> 17) | (h << 15);  // Rotate right 17 bits
                    // 多次重新计算hash模仿多个 hash函数，这里换成多个hash函数也是一样的
                    for (size_t j = 0; j < k_; j++) {
                        // 保证h 的长度不大于bloom过滤器的长度
                        const uint32_t bitpos = h % bits;
                        // 对对应位置进行置位
                        array[bitpos / 8] |= (1 << (bitpos % 8));
                        // 更新获得一个新的hash数值
                        h += delta;
                    }
                }
            }

            bool KeyMayMatch(const Slice &key, const Slice &bloom_filter) const override {
                const size_t len = bloom_filter.size();
                if (len < 2) return false;

                const char *array = bloom_filter.data();
                // 最后一个byte代表了使用多少hash函数
                // 除最后一个byte之外代表bit数组，详情将CreateFilter函数
                const size_t bits = (len - 1) * 8;

                // Use the encoded k so that we can read filters generated by
                // bloom filters created using different parameters.
                // 存储的是hash函数的个数
                const size_t k = array[len - 1];
                if (k > 30) {
                    // Reserved for potentially new encodings for short bloom filters.
                    // Consider it a match.
                    return true;
                }

                uint32_t h = BloomHash(key);
                const uint32_t delta = (h >> 17) | (h << 15);  // Rotate right 17 bits
                for (size_t j = 0; j < k; j++) {
                    const uint32_t bitpos = h % bits;
                    // (array[bitpos / 8] & (1 << (bitpos % 8)))
                    // 查找对应的bit位是否为0 若为0说明肯定不存在，就直接返回
                    if ((array[bitpos / 8] & (1 << (bitpos % 8))) == 0) return false;
                    h += delta;
                }
                // 只是说明可能存在
                return true;
            }

        private:
            // 平均每个key拥有的bit数目
            size_t bits_per_key_;
            // hash func的个数
            size_t k_;
        };
    }  // namespace

    // 匿名空间的元素只有在同一个源文件中能够之间调用
    const FilterPolicy *sNewBloomFilterPolicy(int bits_per_key) {
        return new BloomFilterPolicy(bits_per_key);
    }

}  // namespace leveldb