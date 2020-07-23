//
// Created by mystic on 2020/4/28.
//

#include "host_api.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *GetHostAPICall(int iIndex) {
    return g_Script.HostAPICallTable.ppstrCalls[iIndex];
}

int RegisterHostFunc(char *ptrName, int iCount, func ptrFunc) {
    HostFunc *table = g_Script.HostAPICallTable.ptrHostFuncTable;
    if (g_Script.HostAPICallTable.iFuncNum >= MAXHOSTFUNC)
        return -1;
    printf("table index;%d\n", g_Script.HostAPICallTable.iFuncNum);
    printf("p;%p\n", table[g_Script.HostAPICallTable.iFuncNum].ptrFuncName);
    table[g_Script.HostAPICallTable.iFuncNum].fHostfunc = ptrFunc;
    table[g_Script.HostAPICallTable.iFuncNum].iParamCount = iCount;
    table[g_Script.HostAPICallTable.iFuncNum].ptrFuncName = (char *) malloc(strlen(ptrName) + 1);
    strcpy(table[g_Script.HostAPICallTable.iFuncNum].ptrFuncName, ptrName);
    strupr(table[g_Script.HostAPICallTable.iFuncNum].ptrFuncName);
    return g_Script.HostAPICallTable.iFuncNum++;
}

char *GetParamAsString(int iParamIndex) {
    int iTopIndex = g_Script.Stack.iTopIndex;
    Value Param = g_Script.Stack.pElmnts[iTopIndex - (iParamIndex + 1)];
    return CoerceValueToString(Param);
}

Value GetParamAsValue(int iParamIndex) {
    int iTopIndex = g_Script.Stack.iTopIndex;
    return g_Script.Stack.pElmnts[iTopIndex - (iParamIndex + 1)];
}


int ReturnIntFromHost(int iRet, int iParamCount) {
    g_Script.Stack.iTopIndex -= iParamCount;
    g_Script._RetVal.iType = OP_TYPE_INT;
    g_Script._RetVal.iIntLiteral = iRet;
}

int ReturnStringFromHost(char *ch, int iParamCount) {
    g_Script.Stack.iTopIndex -= iParamCount;
    Value val;
    val.iType = OP_TYPE_STRING;
    val.pstrStringLiteral = ch;
    CopyValue(&g_Script._RetVal, val);
}

void strupr(char *pstrString) {
    while (*pstrString) {
        if (*pstrString >= 'a' && *pstrString <= 'z') {
            *pstrString = *pstrString - 32;
        }
        pstrString++;
    }
}