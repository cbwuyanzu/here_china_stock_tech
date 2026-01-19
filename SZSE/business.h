//
// Created by chend on 2026/1/19.
//

#ifndef SZSE_BUSINESS_H
#define SZSE_BUSINESS_H

#include "types.h"

void popAndParse(int timeoutMs);

int deserialize300111(RawSzMDData &md,const void* buffer, uint32_t length);

void showMdData(const RawSzMDData & md);

void OnRealTimeMD(const RawSzMDData &md);

#endif //SZSE_BUSINESS_H