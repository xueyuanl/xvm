//
// Created by mystic on 2020/4/22.
//

#include "stack.h"
#include <stdio.h>

Value GetStackValue(int iIndex) {
    // Use ResolveStackIndex () to return the element at the specified index
    return g_Script.Stack.pElmnts[ResolveStackIndex (iIndex)];
}

void SetStackValue(int iIndex, Value Val) {
    // Use ResolveStackIndex () to set the element at the specified index
    g_Script.Stack.pElmnts[ResolveStackIndex (iIndex)] = Val;
}

void Push(Value Val) {
    // Get the current top element
    int iTopIndex = g_Script.Stack.iTopIndex;
    // Put the value into the current top index
    g_Script.Stack.pElmnts[iTopIndex] = Val;
    // Increment the top index
    ++g_Script.Stack.iTopIndex;
}

Value Pop() {
    // Decrement the top index to clear the old element for overwriting
    --g_Script.Stack.iTopIndex;
    // Get the current top element
    int iTopIndex = g_Script.Stack.iTopIndex;
    // Use this index to read the top element
    Value Val = g_Script.Stack.pElmnts[iTopIndex];
    // Return the value to the caller
    return Val;
}

void PushFrame(int iSize) {
    // Increment the top index by the size of the frame
    g_Script.Stack.iTopIndex += iSize;
    // Move the frame index to the new top of the stack
    g_Script.Stack.iFrameIndex = g_Script.Stack.iTopIndex;
}

void PopFrame(int iSize) {
    g_Script.Stack.iTopIndex -= iSize;
}

void CopyValue(Value *pDest, Value Source) {  // TODO: why not use the pointer to pass source param
    // If the destination already contains a string, make sure to free it first
    if (pDest->iType == OP_TYPE_STRING)
        free(pDest->pstrStringLiteral);
    // Copy the object
    *pDest = Source;
    // Make a physical copy of the source string, if necessary
    if (Source.iType == OP_TYPE_STRING) {
        pDest->pstrStringLiteral = (char *) malloc(strlen(Source.pstrStringLiteral) + 1);
        strcpy(pDest->pstrStringLiteral, Source.pstrStringLiteral);
    }
}