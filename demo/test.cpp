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

#include <fstream>
#include <iostream>
#include <filesystem>

#include <cstdlib> // 用于Windows上的_mkdir函数
#include <sys/stat.h> // 用于Linux上的mkdir函数

using namespace std;


#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>


int main() {

    char *lpStr = "abcd";

    std::cout << *(lpStr) << std::endl;
    lpStr++;
    std::cout << *(lpStr) << std::endl;
    lpStr++;
    std::cout << *(lpStr) << std::endl;lpStr++;
    std::cout << *(lpStr) << std::endl;



    std::cout << "All files moved successfully." << std::endl;
    return 0;
}