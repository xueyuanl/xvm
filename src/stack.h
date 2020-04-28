//
// Created by mystic on 2020/4/22.
//

#ifndef XVM_STACK_H
#define XVM_STACK_H

#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "constants.h"

#define ResolveStackIndex(iIndex) ( iIndex < 0 ? iIndex += g_Script.Stack.iFrameIndex : iIndex )

Value GetStackValue(int iIndex);

void SetStackValue(int iIndex, Value Val);

void CopyValue(Value *pDest, Value Source);

void Push(Value Val);

Value Pop();

void PushFrame(int iSize);

void PopFrame(int iSize);

#endif //XVM_STACK_H
