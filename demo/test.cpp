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
#include "leveldb/write_batch.h"
#include "leveldb/db.h"



using namespace std;



std::map<int32_t, std::string> GetDatabase() {

    std::map<int32_t, std::string> temp;


    temp[0] = "123";
    temp[1] = "123";
    temp[2] = "123";
    temp[3] = "123";
    temp[4] = "123";
    temp[5] = "123";
    temp[6] = "123";
    temp[7] = "123";
    return temp;
}



int main(int argc, char *argv[]) {

    std::string * lpStr = &GetDatabase()[2];


   std::cout << *lpStr <<  lpStr->size() << std::endl;

    return 0;
}
