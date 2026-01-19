//
// Created by chend on 2026/1/14.
//

#ifndef SZSE_SZMDPARSER_H
#define SZSE_SZMDPARSER_H

#include <mutex>
#include <unordered_map>

#include "types.h"


class SZMDParser {
    std::unordered_map<int,MyMDItem> myMDMap;
    std::mutex mtx;
public:
    SZMDParser();

    int makeKey(int marketCode, int stockCode);

    MyMDItem makeElement(int marketCode, int stockCode);

    int savePx(int marketCode, int stockCode, int entryType, int value);
    int loadPx(int marketCode, int stockCode, int entryType, int& value);
    int saveOrigTime(int marketCode, int stockCode, long long origTime);
    int saveMktStkCode(int marketCode, int stockCode);
    int parse(const RawSzMDData& mdData);
    void show();
    void dump(const char* fileName);
};

#endif //SZSE_SZMDPARSER_H