//
// Created by pat on 4/8/20.
//

#ifndef XVM_STRUCTURE_H
#define XVM_STRUCTURE_H

typedef struct _Value               // A runtime value
{
    int iType;                      // Type
    union                           // The value
    {
        int iIntLiteral;            // Integer literal
        float fFloatLiteral;        // Float literal
        char *pstrStringLiteral;    // String literal
        int iStackIndex;            // Stack Index
        int iInstrIndex;            // Instruction index
        int iFuncIndex;             // Function index
        int iHostAPICallIndex;      // Host API Call index
        int iReg;                   // Register code
    };
    int iOffsetIndex;               // Index of the offset
} Value;

typedef struct _Instr               // An instruction
{
    int iOpCode;                    // The opCode
    int iOpCount;                   // The number of operands
    Value *pOpList;                 // The operand list
} Instr;

typedef struct _InstrStream         // An instruction stream
{
    Instr *pInstrs;                 // The instructions themselves
    int iSize;                      // The number of instructions in the stream
    int iCurrInstr;                 // The instruction pointer
} InstrStream;

typedef struct _RuntimeStack        // A runtime stack
{
    Value *pElmnts;                 // The stack elements
    int iSize;                      // The number of elements in the stack
    int iTopIndex;                  // The top index
    int iFrameIndex;                // Index of the top of the current stack frame.
} RuntimeStack;

typedef struct _Func                // Function table element
{
    int iEntryPoint;                // The entry point
    int iParamCount;                // Number of parameters to expect
    int iLocalDataSize;             // Total size of all local data
    // Notice again that even though the StackFrameSize element is always defined as
    //ParamCount + 1 + LocalDataSize
    int iStackFrameSize;            // Total size of the stack frame
} Func;

typedef struct _HostAPICallTable    // A host API call table
{
    char **ppstrCalls;              // Pointer to the call array
    int iSize;                      // The number of calls in the array
} HostAPICallTable;

typedef struct _Script              // Encapsulates a full script
{
    // Header data
    int iGlobalDataSize;            // The size of the script's global data
    int iIsMainFuncPresent;         // Is _Main () present?
    int iMainFuncIndex;             // _Main ()'s function index
    int iIsPaused;                  // Is the script currently paused?
    int iPauseEndTime;              // If so, when should it resume?
    // Register file
    Value _RetVal;                  // The _RetVal register
    // Script data
    InstrStream InstrStream;        // The instruction stream
    RuntimeStack Stack;             // The runtime stack
    Func *pFuncTable;               // The function table
    HostAPICallTable HostAPICallTable; // The host API call table
} Script;

#endif //XVM_STRUCTURE_H
