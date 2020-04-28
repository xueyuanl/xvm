//
// Created by mystic on 2020/4/28.
//

#include "host_api.h"

char *GetHostAPICall(int iIndex) {
    return g_Script.HostAPICallTable.ppstrCalls[iIndex];
}