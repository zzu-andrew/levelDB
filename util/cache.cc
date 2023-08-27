// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "leveldb/cache.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "port/port.h"
#include "port/thread_annotations.h"
#include "util/hash.h"
#include "util/mutexlock.h"

// 经典永不过时  https://zhuanlan.zhihu.com/p/511274035

namespace leveldb {

    Cache::~Cache() = default;

    namespace {

// LRU cache implementation
// 入口使用一个in_cache bool类型变量来指定。
// Cache entries have an "in_cache" boolean indicating whether the cache has a
// reference on the entry.  The only ways that this can become false without the
// entry being passed to its "deleter" are via Erase(), via Insert() when
// an element with a duplicate key is inserted, or on destruction of the cache.
//
// The cache keeps two linked lists of items in the cache.  All items in the
// cache are in one list or the other, and never both.  Items still referenced
// by clients but erased from the cache are in neither list.  The lists are:
// - in-use:  contains the items currently referenced by clients, in no
//   particular order.  (This list is used for invariant checking.  If we
//   removed the check, elements that would otherwise be on this list could be
//   left as disconnected singleton lists.)
// - LRU:  contains the items not currently referenced by clients, in LRU order
// Elements are moved between these lists by the Ref() and Unref() methods,
// when they detect an element in the cache acquiring or losing its only
// external reference.
// LRU顺序链表中根据调用来更新引用关系
// An entry is a variable length heap-allocated structure.  Entries
// are kept in a circular doubly linked list ordered by access time.
// hash 桶的元素
// 元素结构定义，用来存储KVMap值
        struct LRUHandle {
            // 元素节点指向的Value值， 这样一个LRUHandle中就能存储一对键值对了
            void *value;
            //  value是调用者申请的，需要调用者提供清理函数
            void (*deleter)(const Slice &, void *value);
            // 单向链表时指向下一个节点
            LRUHandle *next_hash;
            // 双向链表时时使用
            LRUHandle *next;
            LRUHandle *prev;

            size_t charge;  // TODO(opt): Only allow uint32_t?
            size_t key_length;
            bool in_cache;     // Whether entry is in the cache.
            uint32_t refs;     // References, including cache reference, if present.
            // hash值
            uint32_t hash;     // Hash of key(); used for fast sharding and comparisons
            char key_data[1];  // Beginning of key，其实就是相当于一个指针，起到一个定位的作用
            // 将当前LruCache转换为Slice
            Slice key() const {
                // next_ is only equal to this if the LRU handle is the list head of an
                // empty list. List heads never have meaningful keys.
                assert(next != this);

                return Slice(key_data, key_length);
            }
        };

// We provide our own simple hash table since it removes a whole bunch
// of porting hacks and is also faster than some of the built-in hash
// table implementations in some of the compiler/runtime combinations
// we have tested.  E.g., readrandom speeds up by ~5% over the g++
// 4.4.3's builtin hashtable.
// 没有使用内置的hashTable是因为这个 HashTable足够的快
        class HandleTable {
        public:
            HandleTable() : length_(0), elems_(0), list_(nullptr) { Resize(); }

            ~HandleTable() { delete[] list_; }

            LRUHandle *Lookup(const Slice &key, uint32_t hash) {
                return *FindPointer(key, hash);
            }

            LRUHandle *Insert(LRUHandle *h) {
                //
                LRUHandle **ptr = FindPointer(h->key(), h->hash);
                LRUHandle *old = *ptr;
                h->next_hash = (old == nullptr ? nullptr : old->next_hash);
                *ptr = h;
                if (old == nullptr) {
                    ++elems_;
                    if (elems_ > length_) {
                        // Since each cache entry is fairly large, we aim for a small
                        // average linked list length (<= 1).
                        Resize();
                    }
                }
                return old;
            }

            LRUHandle *Remove(const Slice &key, uint32_t hash) {
                LRUHandle **ptr = FindPointer(key, hash);
                LRUHandle *result = *ptr;
                if (result != nullptr) {
                    // 如果非空则将当前节点指向下一个节点，同时当期那节点可以的指针同时被移除
                    *ptr = result->next_hash;
                    --elems_;
                }
                return result;
            }

        private:
            // The table consists of an array of buckets where each bucket is
            // a linked list of cache entries that hash into the bucket.
            // hash 桶长度
            uint32_t length_;
            // hash 中的元素个数
            uint32_t elems_;
            // hash桶
            LRUHandle **list_;

            // Return a pointer to slot that points to a cache entry that
            // matches key/hash.  If there is no such cache entry, return a
            // pointer to the trailing slot in the corresponding linked list.
            // 同样的hash值只能存在桶上的固定位置，
            // 1. 使用hash值定位到hash链表的头指针存储位置
            // 2. 如果 通过slice 重载的== != 来判断hash和key值是否相同，因为不同的hash经过length处理之后还是会得出同样的结果(hash 冲撞)
            LRUHandle **FindPointer(const Slice &key, uint32_t hash) {
                // list hash桶，桶装LRUHandle 元素
                LRUHandle **ptr = &list_[hash & (length_ - 1)];
                while (*ptr != nullptr && ((*ptr)->hash != hash || key != (*ptr)->key())) {
                    ptr = &(*ptr)->next_hash;
                }
                return ptr;
            }

            void Resize() {
                uint32_t new_length = 4;
                // 每次增长长度按2的倍数并且保证大于元素的个数
                while (new_length < elems_) {
                    new_length *= 2;
                }
                // hash桶中存储的是hash元素的链表指针
                LRUHandle **new_list = new LRUHandle *[new_length];
                memset(new_list, 0, sizeof(new_list[0]) * new_length);
                uint32_t count = 0;
                // 将原hash桶中的元素，遍历到新的hash桶中
                // 插入的过程中采用头插法
                for (uint32_t i = 0; i < length_; i++) {
                    LRUHandle *h = list_[i];
                    while (h != nullptr) {
                        // ？？？？？ 扩充之后同样的hash
                        LRUHandle *next = h->next_hash;
                        uint32_t hash = h->hash;
                        LRUHandle **ptr = &new_list[hash & (new_length - 1)];
                        // 采用头插法，首次 *ptr为null,
                        // 非首次则h 指向上次*ptr指向的内容， *ptr本身赋值代表它所指向的内容
                        h->next_hash = *ptr;
                        // 将h放入新的hash列表
                        *ptr = h;
                        // h指向下一个元素
                        h = next;
                        count++;
                    }
                }
                // 去报所有的hash值都放到新的hash桶中
                assert(elems_ == count);
                // 释放原有hash桶
                delete[] list_;
                list_ = new_list;
                length_ = new_length;
            }
        };

// A single shard of sharded cache.
        class LRUCache {
        public:
            LRUCache();

            ~LRUCache();

            // Separate from constructor so caller can easily make an array of LRUCache
            void SetCapacity(size_t capacity) { capacity_ = capacity; }

            // Like Cache methods, but with an extra "hash" parameter.
            Cache::Handle *Insert(const Slice &key, uint32_t hash, void *value,
                                  size_t charge,
                                  void (*deleter)(const Slice &key, void *value));

            Cache::Handle *Lookup(const Slice &key, uint32_t hash);

            void Release(Cache::Handle *handle);

            void Erase(const Slice &key, uint32_t hash);

            void Prune();

            size_t TotalCharge() const {
                MutexLock l(&mutex_);
                return usage_;
            }

        private:
            static void LRU_Remove(LRUHandle *e);

            static void LRU_Append(LRUHandle *list, LRUHandle *e);

            void Ref(LRUHandle *e);

            void Unref(LRUHandle *e);

            bool FinishErase(LRUHandle *e) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

            // 需要在调用之前进行初始化
            size_t capacity_;

            // mutex_ 用于保护下面Usage状态的锁.
            // clang 使用时支持死锁检测
            mutable port::Mutex mutex_;
            // GUARDED_BY 这里告诉编译器当更改usage_的时候需要使用对应的锁保护
            // 如果没有使用指定的锁保护会进行报错
            size_t usage_ GUARDED_BY(mutex_);

            // Dummy head of LRU list.
            // lru.prev is newest entry, lru.next is oldest entry.
            // Entries have refs==1 and in_cache==true.
            LRUHandle lru_ GUARDED_BY(mutex_);

            // Dummy head of in-use list.
            // Entries are in use by clients, and have refs >= 2 and in_cache==true.
            LRUHandle in_use_ GUARDED_BY(mutex_);

            HandleTable table_ GUARDED_BY(mutex_);
        };

        LRUCache::LRUCache() : capacity_(0), usage_(0) {
            // Make empty circular linked lists.
            // 初始化时，按照空环进行初始化
            lru_.next = &lru_;
            lru_.prev = &lru_;
            // 正在被使用的元素
            in_use_.next = &in_use_;
            in_use_.prev = &in_use_;
        }

        LRUCache::~LRUCache() {
            assert(in_use_.next == &in_use_);  // Error if caller has an unreleased handle
            for (LRUHandle *e = lru_.next; e != &lru_;) {
                LRUHandle *next = e->next;
                assert(e->in_cache);
                e->in_cache = false;
                assert(e->refs == 1);  // Invariant of lru_ list.
                Unref(e);
                e = next;
            }
        }

        void LRUCache::Ref(LRUHandle *e) {
            if (e->refs == 1 && e->in_cache) {  // If on lru_ list, move to in_use_ list.
                LRU_Remove(e);
                LRU_Append(&in_use_, e);
            }
            // 引用增加
            e->refs++;
        }

        void LRUCache::Unref(LRUHandle *e) {
            assert(e->refs > 0);
            e->refs--;
            if (e->refs == 0) {  // Deallocate.
                assert(!e->in_cache);
                (*e->deleter)(e->key(), e->value);
                free(e);
            } else if (e->in_cache && e->refs == 1) {
                // No longer in use; move to lru_ list.
                //lru_ 表示没有在使用的节点
                LRU_Remove(e);
                LRU_Append(&lru_, e);
            }
        }

        void LRUCache::LRU_Remove(LRUHandle *e) {
            // 将当前节点剔除
            e->next->prev = e->prev;
            e->prev->next = e->next;
        }

        void LRUCache::LRU_Append(LRUHandle *list, LRUHandle *e) {
            // Make "e" newest entry by inserting just before *list
            e->next = list;
            e->prev = list->prev;
            e->prev->next = e;
            e->next->prev = e;
        }

        Cache::Handle *LRUCache::Lookup(const Slice &key, uint32_t hash) {
            MutexLock l(&mutex_);
            // 如果存在增加引用
            LRUHandle *e = table_.Lookup(key, hash);
            if (e != nullptr) {
                Ref(e);
            }
            return reinterpret_cast<Cache::Handle *>(e);
        }

        void LRUCache::Release(Cache::Handle *handle) {
            MutexLock l(&mutex_);
            Unref(reinterpret_cast<LRUHandle *>(handle));
        }

        Cache::Handle *LRUCache::Insert(const Slice &key, uint32_t hash, void *value,
                                        size_t charge,
                                        void (*deleter)(const Slice &key,
                                                        void *value)) {
            MutexLock l(&mutex_);
            // char key_data[1]; -1 是为了修正 key_data占用的大小
            auto *e =
                    reinterpret_cast<LRUHandle *>(malloc(sizeof(LRUHandle) - 1 + key.size()));
            e->value = value;
            e->deleter = deleter;
            e->charge = charge;
            e->key_length = key.size();
            e->hash = hash;
            e->in_cache = false;
            e->refs = 1;  // for the returned handle.
            // key_data这里作用就是在一段内存中取个位置的作用，这个位置必须是对应结构体的最后一个元素
            std::memcpy(e->key_data, key.data(), key.size());

            if (capacity_ > 0) {
                e->refs++;  // for the cache's reference.
                e->in_cache = true;
                LRU_Append(&in_use_, e);
                usage_ += charge;
                FinishErase(table_.Insert(e));
            } else {  // don't cache. (capacity_==0 is supported and turns off caching.)
                // 如果capacity为空，不对插入的数据进行缓存
                // next is read by key() in an assert, so it must be initialized
                e->next = nullptr;
            }
            while (usage_ > capacity_ && lru_.next != &lru_) {
                LRUHandle *old = lru_.next;
                assert(old->refs == 1);
                bool erased = FinishErase(table_.Remove(old->key(), old->hash));
                if (!erased) {  // to avoid unused variable when compiled NDEBUG
                    assert(erased);
                }
            }

            return reinterpret_cast<Cache::Handle *>(e);
        }

// If e != nullptr, finish removing *e from the cache; it has already been
// removed from the hash table.  Return whether e != nullptr.
        bool LRUCache::FinishErase(LRUHandle *e) {
            if (e != nullptr) {
                // 在缓存中缓存着
                assert(e->in_cache);
                LRU_Remove(e);
                // 剔除数据
                e->in_cache = false;
                usage_ -= e->charge;
                // 如果e->in_cache 为false 进入Unref只是将引用计数 - 1
                Unref(e);
            }
            return e != nullptr;
        }

        void LRUCache::Erase(const Slice &key, uint32_t hash) {
            MutexLock l(&mutex_);
            FinishErase(table_.Remove(key, hash));
        }

        void LRUCache::Prune() {
            MutexLock l(&mutex_);
            while (lru_.next != &lru_) {
                LRUHandle *e = lru_.next;
                assert(e->refs == 1);
                bool erased = FinishErase(table_.Remove(e->key(), e->hash));
                if (!erased) {  // to avoid unused variable when compiled NDEBUG
                    assert(erased);
                }
            }
        }
        // 匿名空间中定义变量不需要添加static外部也引用不了，直接使用 const就行了
        const int kNumShardBits = 4;
        const int kNumShards = 1 << kNumShardBits;
        // 将LRU再次分层，按照hash的后四位分到不同的LRUCache中
        class ShardedLRUCache : public Cache {
        private:
            LRUCache shard_[kNumShards];
            port::Mutex id_mutex_;
            uint64_t last_id_;

            static inline uint32_t HashSlice(const Slice &s) {
                return Hash(s.data(), s.size(), 0);
            }

            static uint32_t Shard(uint32_t hash) { return hash >> (32 - kNumShardBits); }

        public:
            explicit ShardedLRUCache(size_t capacity) : last_id_(0) {
                const size_t per_shard = (capacity + (kNumShards - 1)) / kNumShards;
                for (auto & s : shard_) {
                    s.SetCapacity(per_shard);
                }
              }

            ~ShardedLRUCache() override = default;

            Handle *Insert(const Slice &key, void *value, size_t charge,
                           void (*deleter)(const Slice &key, void *value)) override {
                const uint32_t hash = HashSlice(key);
                return shard_[Shard(hash)].Insert(key, hash, value, charge, deleter);
            }

            Handle *Lookup(const Slice &key) override {
                const uint32_t hash = HashSlice(key);
                return shard_[Shard(hash)].Lookup(key, hash);
            }

            void Release(Handle *handle) override {
                auto *h = reinterpret_cast<LRUHandle *>(handle);
                shard_[Shard(h->hash)].Release(handle);
            }

            void Erase(const Slice &key) override {
                const uint32_t hash = HashSlice(key);
                shard_[Shard(hash)].Erase(key, hash);
            }

            void *Value(Handle *handle) override {
                return reinterpret_cast<LRUHandle *>(handle)->value;
            }

            uint64_t NewId() override {
                MutexLock l(&id_mutex_);
                return ++(last_id_);
            }

            void Prune() override {
                for (auto & s : shard_) {
                    s.Prune();
                }
            }

            size_t TotalCharge() const override {
                size_t total = 0;
                for (const auto & s : shard_) {
                    total += s.TotalCharge();
                }
                return total;
            }
        };

    }  // end anonymous namespace

    Cache *NewLRUCache(size_t capacity) { return new ShardedLRUCache(capacity); }

}  // namespace leveldb
