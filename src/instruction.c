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

Value *GetOpValue(int iOpIndex) {
    // Get the current instruction
    int iCurrInstr = g_Script.InstrStream.iCurrInstr;

    // Get the operand value
    return &g_Script.InstrStream.pInstrs[iCurrInstr].pOpList[iOpIndex];
}


int GetOpAsInt(int iOpIndex) {
    // Get the current instruction
    int iCurrInstr = g_Script.InstrStream.iCurrInstr;
    // Return the type
    return g_Script.InstrStream.pInstrs[iCurrInstr].pOpList[iOpIndex].iIntLiteral;
}

int ResolveOpType(int iOpIndex) {
    // Resolve the operand's value
    Value OpValue = ResolveOpValue(iOpIndex);
    // Return the value type
    return OpValue.iType;
}

Value ResolveOpValue(int iOpIndex) {
    // Get the current instruction
    int iCurrInstr = g_Script.InstrStream.iCurrInstr;

    // Get the operand type
    Value OpValue = g_Script.InstrStream.pInstrs[iCurrInstr].pOpList[iOpIndex];

    // Determine what to return based on the value's type
    switch (OpValue.iType) {
        // It's a stack index so resolve it
        case OP_TYPE_ABS_STACK_INDEX:
        case OP_TYPE_REL_STACK_INDEX: {
            // Resolve the index and use it to return the corresponding stack element
            int iAbsIndex = ResolveOpStackIndex(iOpIndex);
            return GetStackValue(iAbsIndex);
        }
            // It's in _RetVal
        case OP_TYPE_REG:
            return g_Script._RetVal;
            // Anything else can be returned as-is
        default:
            return OpValue;
    }
}

int ResolveOpStackIndex(int iOpIndex) {
    // Get the current instruction
    int iCurrInstr = g_Script.InstrStream.iCurrInstr;
    // Get the operand type type
    Value OpValue = g_Script.InstrStream.pInstrs[iCurrInstr].pOpList[iOpIndex];
    // Resolve the stack index based on its type
    switch (OpValue.iType) {
        // It's an absolute index so return it as-is
        case OP_TYPE_ABS_STACK_INDEX:
            return OpValue.iStackIndex;
            // It's a relative index so resolve it
        case OP_TYPE_REL_STACK_INDEX: {
            // First get the base index
            int iBaseIndex = OpValue.iStackIndex;
            // Now get the index of the variable
            int iOffsetIndex = OpValue.iOffsetIndex;
            // Get the variable's value
            Value StackValue = GetStackValue(iOffsetIndex);
            // Now add the variable's integer field to the base index to produce the
            // absolute index
            return iBaseIndex + StackValue.iIntLiteral;
        }
            // Return zero for everything else, but we shouldn't encounter this case
        default:
            return 0;
    }
}

Value GetStackValue(int iIndex) {
    // Use ResolveStackIndex () to return the element at the specified index
    return g_Script.Stack.pElmnts[ResolveStackIndex(iIndex)];
}

inline int ResolveOpAsInt(int iOpIndex) {
    // Resolve the operand's value
    Value OpValue = ResolveOpValue(iOpIndex);
    // Coerce it to an int and return it
    int iInt = CoerceValueToInt(OpValue);
    return iInt;
}

float ResolveOpAsFloat(int iOpIndex) {
    // Resolve the operand's value
    Value OpValue = ResolveOpValue(iOpIndex);
    // Coerce it to a float and return it
    float fFloat = CoerceValueToFloat(OpValue);
    return fFloat;
}


char *ResolveOpAsString(int iOpIndex) {
    // Resolve the operand's value
    Value OpValue = ResolveOpValue(iOpIndex);
    // Coerce it to a string and return it
    char *pstrString = CoerceValueToString(OpValue);
    return pstrString;
}

int CoerceValueToInt(Value Val) {
    // Determine which type the Value currently is
    switch (Val.iType) {
        // It's an integer, so return it as-is
        case OP_TYPE_INT:
            return Val.iIntLiteral;
            // It's a float, so cast it to an integer
        case OP_TYPE_FLOAT:
            return (int) Val.fFloatLiteral;
            // It's a string, so convert it to an integer
        case OP_TYPE_STRING:
            return atoi(Val.pstrStringLiteral);
            // Anything else is invalid
        default:
            return 0;
    }
}

float CoerceValueToFloat(Value Val) {
    // Determine which type the Value currently is
    switch (Val.iType) {
        // It's an integer, so cast it to a float
        case OP_TYPE_INT:
            return (float) Val.iIntLiteral;
            // It's a float, so return it as-is
        case OP_TYPE_FLOAT:
            return Val.fFloatLiteral;
            // It's a string, so convert it to a float
        case OP_TYPE_STRING:
            return (float) atof(Val.pstrStringLiteral);
            // Anything else is invalid
        default:
            return 0;
    }
}

char *CoerceValueToString(Value Val) {
    char *pstrCoercion = NULL;
    if (Val.iType != OP_TYPE_STRING)
        pstrCoercion = (char *) malloc(MAX_COERCION_STRING_SIZE + 1);
    // Determine which type the Value currently is
    switch (Val.iType) {
        // It's an integer, so convert it to a string
        case OP_TYPE_INT:
            itoa(Val.iIntLiteral, pstrCoercion, 10);
            return pstrCoercion;
            // It's a float, so use sprintf () to convert it since there's
            // no built-in function for converting floats to strings
        case OP_TYPE_FLOAT:
            sprintf(pstrCoercion, "%f", Val.fFloatLiteral);
            return pstrCoercion;
            // It's a string, so return it as-is
        case OP_TYPE_STRING:
            return Val.pstrStringLiteral;
            // Anything else is invalid
        default:
            return NULL;
    }
}

Value *ResolveOpPntr(int iOpIndex) {
    // Get the method of indirection
    int iIndirMethod = GetOpType(iOpIndex);
    // Return a pointer to wherever the operand lies
    switch (iIndirMethod) {
        // It's on the stack
        case OP_TYPE_ABS_STACK_INDEX:
        case OP_TYPE_REL_STACK_INDEX: {
            int iStackIndex = ResolveOpStackIndex(iOpIndex);
            return &g_Script.Stack.pElmnts[ResolveStackIndex(iStackIndex)];
        }
            // It's _RetVal
        case OP_TYPE_REG:
            return &g_Script._RetVal;
    }
    // Return NULL for anything else
    return NULL;
}

