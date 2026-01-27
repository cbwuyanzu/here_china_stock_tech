//
// Created by chend on 2026/1/15.
//

#ifndef SZSE_THREADFUNCS_H
#define SZSE_THREADFUNCS_H
#include "myInih.h"
#include "utility.h"
#include "session.h"

void szIoThreadFunc(const Configuration &cfg);

void szBusinessThreadFunc(const Configuration &cfg);

#endif //SZSE_THREADFUNCS_H