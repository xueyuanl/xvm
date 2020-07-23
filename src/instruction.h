//
// Created by hsuehyuan li on 2020-04-12.
//

#ifndef XVM_INSTRUCTION_H
#define XVM_INSTRUCTION_H

#include "global.h"
#include "constants.h"
#include "stack.h"
#include <stdlib.h>
#include <stdio.h>
#include "host_api.h"

#define INSTRS_ARRAY         {    \
            "Mov",              \
            "Add", "Sub", "Mul", "Div", "Mod", "Exp", "Neg", "Inc", "Dec",\
            "And", "Or", "XOr", "Not", "ShL", "ShR",    \
            "Concat", "GetChar", "SetChar",     \
            "Jmp", "JE", "JNE", "JG", "JL", "JGE", "JLE",   \
            "Push", "Pop",  \
            "Call", "Ret", "CallHost",  \
            "Pause", "Exit" , "Sqrt"         \
        }

// ---- Instruction Opcodes -----------------------------------------------------------

#define INSTR_MOV               0

#define INSTR_ADD               1
#define INSTR_SUB               2
#define INSTR_MUL               3
#define INSTR_DIV               4
#define INSTR_MOD               5
#define INSTR_EXP               6
#define INSTR_NEG               7
#define INSTR_INC               8
#define INSTR_DEC               9

#define INSTR_AND               10
#define INSTR_OR                11
#define INSTR_XOR               12
#define INSTR_NOT               13
#define INSTR_SHL               14
#define INSTR_SHR               15

#define INSTR_CONCAT            16
#define INSTR_GETCHAR           17
#define INSTR_SETCHAR           18

#define INSTR_JMP               19
#define INSTR_JE                20
#define INSTR_JNE               21
#define INSTR_JG                22
#define INSTR_JL                23
#define INSTR_JGE               24
#define INSTR_JLE               25

#define INSTR_PUSH              26
#define INSTR_POP               27

#define INSTR_CALL              28
#define INSTR_RET               29
#define INSTR_CALLHOST          30

#define INSTR_PAUSE             31
#define INSTR_EXIT              32

//new
#define INSTR_SQRT              33
#define INSTR_ROL               34

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

int GetOpType(int iOpIndex);

Value *GetOpValue(int iOpIndex);

int ResolveOpType(int iOpIndex);

Value ResolveOpValue(int iOpIndex);

int ResolveOpStackIndex(int iOpIndex);

int ResolveOpAsInt(int iOpIndex);

float ResolveOpAsFloat(int iOpIndex);

char *ResolveOpAsString(int iOpIndex);

int ResolveOpAsInstrIndex(int iOpIndex);

int ResolveOpAsFuncIndex(int iOpIndex);

char *ResolveOpAsHostAPICall(int iOpIndex);

Value *ResolveOpPntr(int iOpIndex);

int CoerceValueToInt(Value Val);

float CoerceValueToFloat(Value Val);

char *CoerceValueToString(Value Val);

Value *ResolveOpPntr(int iOpIndex);

void itoa(int n, char s[]);

#endif //XVM_INSTRUCTION_H
