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

using namespace std;



const uint64_t CREATE_SQL_STR_LEN = 1024 * 1024;

int main(int argc, char *argv[]) {


    struct sql {
        sql() {std::cout << "==========" << std::endl;}
        uint64_t data;
        int sql_;
        char name[CREATE_SQL_STR_LEN];
        char tea;
    };

    typename std::aligned_storage<sizeof(sql),
            alignof(sql)>::type instance_storage_;


    return 0;
}
