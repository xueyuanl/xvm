//
// Created by mystic on 2020/4/28.
//

#ifndef XVM_HOST_API_H
#define XVM_HOST_API_H

#include "global.h"
#include "constants.h"
#include "instruction.h"

char *GetHostAPICall(int iIndex);

int RegisterHostFunc(char *ptrName, int iCount, func ptrFunc);

void strupr(char *pstrString);

char *GetParamAsString(int iParamIndex);

Value GetParamAsValue(int iParamIndex);

int ReturnIntFromHost(int iRet, int iParamCount);

int ReturnStringFromHost(char *ch, int iParamCount);

#endif //XVM_HOST_API_H
