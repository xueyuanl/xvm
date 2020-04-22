//
// Created by hsuehyuan li on 2020-04-12.
//

#ifndef XVM_INSTRUCTION_H
#define XVM_INSTRUCTION_H

#include "global.h"
#include "constants.h"
#include <stdlib.h>
#include <stdio.h>

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

int ResolveOpStackIndex(int iOpIndex);

Value GetStackValue(int iIndex);

int ResolveOpAsInt ( int iOpIndex );

float ResolveOpAsFloat ( int iOpIndex );

char * ResolveOpAsString ( int iOpIndex );

int ResolveOpAsInstrIndex ( int iOpIndex );

int ResolveOpAsFuncIndex ( int iOpIndex );

char * ResolveOpAsHostAPICall ( int iOpIndex );

Value * ResolveOpPntr ( int iOpIndex );

int CoerceValueToInt(Value Val);

float CoerceValueToFloat(Value Val);

char *CoerceValueToString(Value Val);

Value *ResolveOpPntr(int iOpIndex);


#endif //XVM_INSTRUCTION_H
