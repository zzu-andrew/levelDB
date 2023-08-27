//
// Created by wangyz38535 on 2021/10/21.
//

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>

using namespace std;

const uint64_t CREATE_SQL_STR_LEN = 1024 * 1024;

int main(int argc, char *argv[]) {

    char lpCreateSQL[CREATE_SQL_STR_LEN];
    memset(lpCreateSQL, 0 , sizeof(lpCreateSQL));

    auto nRet = snprintf(lpCreateSQL, CREATE_SQL_STR_LEN, "%s", "create virtual table ");

    nRet += snprintf(lpCreateSQL + nRet, CREATE_SQL_STR_LEN- nRet, "%s", "create virtual table ");
    nRet += snprintf(lpCreateSQL + nRet, CREATE_SQL_STR_LEN- nRet, "%s", "create virtual table ");


    return 0;
}
