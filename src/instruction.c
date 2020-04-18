//
// Created by hsuehyuan li on 2020-04-12.
//

#include "instruction.h"

int GetOpType(int iOpIndex) {
    // Get the current instruction
    int iCurrInstr = g_Script.InstrStream.iCurrInstr;
    // Return the type
    return g_Script.InstrStream.pInstrs[iCurrInstr].pOpList[iOpIndex].iType;
}

int GetOpAsInt(int iOpIndex) {
    // Get the current instruction
    int iCurrInstr = g_Script.InstrStream.iCurrInstr;
    // Return the type
    return g_Script.InstrStream.pInstrs[iCurrInstr].pOpList[iOpIndex].iIntLiteral;
}



