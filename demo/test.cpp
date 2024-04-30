//
// Created by wangyz38535 on 2021/10/21.
//

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>

#include <type_traits>
#include <utility>
#include <thread>
#include <set>
#include <iostream>
#include <cassert>
#include <functional>
#include <map>
#include <vector>

#include "leveldb/write_batch.h"
#include "leveldb/db.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include <cstdlib> // 用于Windows上的_mkdir函数
#include <sys/stat.h> // 用于Linux上的mkdir函数


#include <optional>
#include <string>
#include <iostream>

using namespace std;


template<typename EnvType>
class SingletonEnv {
public:
    SingletonEnv() {
        static_assert(sizeof(env_storage_) >= sizeof(EnvType),
                      "env_storage_ will not fit the Env");
        static_assert(alignof(decltype(env_storage_)) >= alignof(EnvType),
                      "env_storage_ does not meet the Env's alignment needs");
        // std::aligned_storage<sizeof(EnvType), alignof(EnvType)>::type 保证了内存大小刚好是 EnvType 大小
        new(&env_storage_) EnvType();
    }

    ~SingletonEnv() = default;

    SingletonEnv(const SingletonEnv &) = delete;

    SingletonEnv &operator=(const SingletonEnv &) = delete;

    Env *env() { return reinterpret_cast<Env *>(&env_storage_); }

    static void AssertEnvNotInitialized() {
    }

private:
    typename std::aligned_storage<sizeof(EnvType), alignof(EnvType)>::type
            env_storage_;
};





int main() {
    Derived derived;
    derived.a();

    derived.name = 12;



    return 0;
}