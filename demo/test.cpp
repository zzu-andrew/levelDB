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


using namespace std;


const uint64_t CREATE_SQL_STR_LEN = 1024 * 1024;

int main(int argc, char *argv[]) {


    std::set<std::string> newSet{};
    newSet.insert("3");
    newSet.insert("4");
    newSet.insert("5");
    newSet.insert("7");
    newSet.insert("1");
    newSet.insert("2");
    newSet.insert("2");


    for (auto& s : newSet) {
        std::cout << s << std::endl;
    }






    return 0;
}
