//
// Created by chend on 2026/1/14.
//

#ifndef SZSE_SZMDPARSER_H
#define SZSE_SZMDPARSER_H

#include <vector>

#include "types.h"


class SZMDParser {
    std::vector<MyMDDataItem> myMDDataList;
public:
    SZMDParser();
    int savePxNL(int marketCode, int stockCode, int entryType, int value);
    int loadPxNL(int marketCode, int stockCode, int entryType, int& value);
    int saveOrigTimeNL(int marketCode, int stockCode, long long origTime);
    int saveMktStkCode(int marketCode, int stockCode);
    int parseNL(const RawSzMDData& mdData);
};

#endif //SZSE_SZMDPARSER_H