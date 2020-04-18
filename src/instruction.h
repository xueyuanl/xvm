//
// Created by hsuehyuan li on 2020-04-12.
//

#ifndef XVM_INSTRUCTION_H
#define XVM_INSTRUCTION_H

#include "global.h"

// Return a floating-point literal
float GetOpAsFloat(int OpIndex);

// Return a string literal
char *GetOpAsString(int OpIndex);

// Return a stack index, and automatically resolve relative indices int GetOpAsStackIndex ( int OpIndex );
// Return an instruction index
int GetOpAsInstrIndex(int OpIndex);


// Return a function table index
int GetOpAsFuncIndex(int OpIndex);

// Return a host API call index
char *GetOpAsHostAPICallIndex(int OpIndex);

// Return a register code
char *GetOpAsReg(int OpIndex);

#endif //XVM_INSTRUCTION_H
