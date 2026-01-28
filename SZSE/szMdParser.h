//
// Created by chend on 2026/1/14.
//

#ifndef SZSE_MDPARSER_H
#define SZSE_MDPARSER_H

#include "types.h"
#include "shmStorage.h"


class MDParser {
    ShmStorage myMDmap;

public:
    MDParser();

    int makeKey(int marketCode, int stockCode);

    MyMDItem makeElement(int marketCode, int stockCode);
    MyMDItem* getElement(int marketCode, int stockCode);

    int savePx(int marketCode, int stockCode, int entryType, int value);
    int loadPx(int marketCode, int stockCode, int entryType, int& value);
    int saveOrigTime(int marketCode, int stockCode, long long origTime);
    int saveMktStkCode(int marketCode, int stockCode);
    int parse(const RawSzMDData& mdData);
    int parse(const RawSzHkMDData &mdData);
    void show();
    void dump(const char* fileName);
};

#endif //SZSE_MDPARSER_H