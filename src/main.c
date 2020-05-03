
//
// Created by pat on 4/8/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include "constants.h"
#include "global.h"
#include "stack.h"
#include "instruction.h"
#include "function.h"

char ppstrMnemonics[][12] = INSTRS_ARRAY;
char *point = NULL;

void PrintOpIndir(int iOpIndex) {
    // Get the method of indirection
    int iIndirMethod = GetOpType(iOpIndex);
    // Print it out
    switch (iIndirMethod) {
        // It's _RetVal
        case OP_TYPE_REG:
            printf("_RetVal");
            break;
            // It's on the stack
        case OP_TYPE_ABS_STACK_INDEX:
        case OP_TYPE_REL_STACK_INDEX: {
            int iStackIndex = ResolveOpStackIndex(iOpIndex);
            printf("[ %d ]", iStackIndex);
            break;
        }
    }
}

void PrintOpValue(int iOpIndex) {
    // Resolve the operand's value
    Value Op = ResolveOpValue(iOpIndex);
    // Print it out
    switch (Op.iType) {
        case OP_TYPE_NULL:
            printf("Null");
            break;
        case OP_TYPE_INT:
            printf("%d", Op.iIntLiteral);
            break;
        case OP_TYPE_FLOAT:
            printf("%f", Op.fFloatLiteral);
            break;
        case OP_TYPE_STRING:
            printf("\"%s\"", Op.pstrStringLiteral);
            break;
        case OP_TYPE_INSTR_INDEX:
            printf("%d", Op.iInstrIndex);
            break;
        case OP_TYPE_HOST_API_CALL_INDEX: {
            char *pstrHostAPICall = ResolveOpAsHostAPICall(iOpIndex);
            printf("%s", pstrHostAPICall);
            break;
        }
    }
}

void Init() {
    // ---- Initialize the script structure
    g_Script.iIsMainFuncPresent = FALSE;
    g_Script.iIsPaused = FALSE;
    g_Script.InstrStream.pInstrs = NULL;
    g_Script.Stack.pElmnts = NULL;
    g_Script.pFuncTable = NULL;
    g_Script.HostAPICallTable.ppstrCalls = NULL;

    g_Script.HostAPICallTable.iFuncNum = 0;


    memset(g_Script.HostAPICallTable.ptrHostFuncTable, 0, sizeof(g_Script.HostAPICallTable.ptrHostFuncTable));
    g_Script.HostAPICallTable.iFuncNum = 0;
    RegisterHostFunc("PrintString", 1, PrintString);

}

int LoadScript(char *pstrFilename) {
    FILE *pScriptFile;
    if (!(pScriptFile = fopen(pstrFilename, "rb")))
        return LOAD_ERROR_FILE_IO;

    // Create a buffer to hold the file's ID string
    // (4 bytes + 1 null terminator = 5)
    char *pstrIDString;
    pstrIDString = (char *) malloc(5);


    // Read the string (4 bytes) and append a null terminator
    fread(pstrIDString, 4, 1, pScriptFile);
    pstrIDString[4] = '\0';

    // Compare the data read from the file to the ID string and exit on an error
    // if they don't match
    if (strcmp(pstrIDString, XSE_ID_STRING) != 0)
        return LOAD_ERROR_INVALID_XSE;

    // Free the buffer
    free(pstrIDString);

    // Read the script version (2 bytes total)
    int iMajorVersion = 0,
            iMinorVersion = 0;

    fread(&iMajorVersion, 1, 1, pScriptFile);
    fread(&iMinorVersion, 1, 1, pScriptFile);

    // Validate the version, since this prototype only supports version 0.4 scripts
    if (iMajorVersion != 0 || iMinorVersion != 4)
        return LOAD_ERROR_UNSUPPORTED_VERS;

    // Read the stack size (4 bytes)
    fread(&g_Script.Stack.iSize, 4, 1, pScriptFile);

    // Check for a default stack size request
    if (g_Script.Stack.iSize == 0)
        g_Script.Stack.iSize = DEF_STACK_SIZE;

    // Allocate the runtime stack
    int iStackSize = g_Script.Stack.iSize;
    g_Script.Stack.pElmnts = (Value *)
            malloc(iStackSize * sizeof(Value));

    // Read the global data size (4 bytes)
    fread(&g_Script.iGlobalDataSize, 4, 1, pScriptFile);
    // Check for presence of _Main () (1 byte)
    fread(&g_Script.iIsMainFuncPresent, 1, 1, pScriptFile);
    // Read _Main ()'s function index (4 bytes)
    fread(&g_Script.iMainFuncIndex, 4, 1, pScriptFile);

    // ---- The Instruction Stream ---------------------------------------------------------

    // Read the instruction count (4 bytes)
    fread(&g_Script.InstrStream.iSize, 4, 1, pScriptFile);
    // Allocate the stream
    g_Script.InstrStream.pInstrs = (Instr *)
            malloc(g_Script.InstrStream.iSize * sizeof(Instr));

    for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Script.InstrStream.iSize; ++iCurrInstrIndex) {
        // Read the opcode (2 bytes)
        g_Script.InstrStream.pInstrs[iCurrInstrIndex].iOpCode = 0;
        fread(&g_Script.InstrStream.pInstrs[iCurrInstrIndex].iOpCode, 2, 1, pScriptFile);

        // Read the operand count (1 byte)
        g_Script.InstrStream.pInstrs[iCurrInstrIndex].iOpCount = 0;
        fread(&g_Script.InstrStream.pInstrs[iCurrInstrIndex].iOpCount, 1, 1, pScriptFile);

        int iOpCount = g_Script.InstrStream.pInstrs[iCurrInstrIndex].iOpCount;

        // Allocate space for the operand list in a temporary pointer
        Value *pOpList = NULL;
        if (iOpCount > 0)
            pOpList = (Value *) malloc(iOpCount * sizeof(Value));

        // Read in the operand list (N bytes)
        for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex) {
            // Read in the operand type (1 byte)
            pOpList[iCurrOpIndex].iType = 0;
            // pOpList[iCurrOpIndex].pstrStringLiteral = NULL;
            fread(&pOpList[iCurrOpIndex].iType, 1, 1, pScriptFile);
            switch (pOpList[iCurrOpIndex].iType) {
                // Integer literal
                case OP_TYPE_INT:
                    fread(&pOpList[iCurrOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
                    break;

                    // Floating-point literal
                case OP_TYPE_FLOAT:
                    fread(&pOpList[iCurrOpIndex].fFloatLiteral, sizeof(float), 1, pScriptFile);
                    break;

                    // String index
                case OP_TYPE_STRING:
                    // Since there's no field in the Value structure for string table
                    // indices, read the index into the integer literal field and set
                    // its type to string index
                    fread(&pOpList[iCurrOpIndex].iIntLiteral, sizeof(int),
                          1, pScriptFile);
                    pOpList[iCurrOpIndex].iType = OP_TYPE_STRING;
                    break;

                    // Instruction index
                case OP_TYPE_INSTR_INDEX:
                    fread(&pOpList[iCurrOpIndex].iInstrIndex, sizeof(int), 1, pScriptFile);
                    break;

                    // Absolute stack index
                case OP_TYPE_ABS_STACK_INDEX:
                    fread(&pOpList[iCurrOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
                    break;

                    // Relative stack index
                case OP_TYPE_REL_STACK_INDEX:
                    fread(&pOpList[iCurrOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
                    fread(&pOpList[iCurrOpIndex].iOffsetIndex, sizeof(int), 1, pScriptFile);
                    break;

                    // Function index
                case OP_TYPE_FUNC_INDEX:
                    fread(&pOpList[iCurrOpIndex].iFuncIndex, sizeof(int), 1, pScriptFile);
                    break;

                    // Host API call index
                case OP_TYPE_HOST_API_CALL_INDEX:
                    fread(&pOpList[iCurrOpIndex].iHostAPICallIndex, sizeof(int), 1, pScriptFile);
                    break;

                    // Register
                case OP_TYPE_REG:
                    fread(&pOpList[iCurrOpIndex].iReg, sizeof(int), 1, pScriptFile);
                    break;

            }
        }
        g_Script.InstrStream.pInstrs[iCurrInstrIndex].pOpList = pOpList;

    }

    // ---- The String Table ---------------------------------------------------------

    int iStringTableSize;

    fread(&iStringTableSize, 4, 1, pScriptFile);
    // If the string table exists, read it
    if (iStringTableSize) {
        // Allocate a string table of this size
        char **ppstrStringTable = (char **) malloc(iStringTableSize * sizeof(char *));
        // Read in each string
        int iCurrStringIndex;
        for (iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++iCurrStringIndex) {
            // Read in the string size (4 bytes)
            int iStringSize;
            fread(&iStringSize, 4, 1, pScriptFile);
            // Allocate space for the string plus a null terminator
            char *pstrCurrString = (char *) malloc(iStringSize + 1);
            // Read in the string data (N bytes) and append the null terminator
            fread(pstrCurrString, iStringSize, 1, pScriptFile);
            pstrCurrString[iStringSize] = '\0';
            // Assign the string pointer to the string table
            ppstrStringTable[iCurrStringIndex] = pstrCurrString;
        }

        // Run through each operand in the instruction stream and assign copies
        // of string operands' corresponding string literals
        for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Script.InstrStream.iSize; ++iCurrInstrIndex) {
            // Get the instruction's operand count and a copy of it's operand list
            int iOpCount = g_Script.InstrStream.pInstrs[iCurrInstrIndex].iOpCount;
            Value *pOpList = g_Script.InstrStream.pInstrs[iCurrInstrIndex].pOpList;
            // Loop through each operand
            for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex) {
                // If the operand is a string index, make a local copy of it's corresponding
                // string in the table
                if (pOpList[iCurrOpIndex].iType == OP_TYPE_STRING) {
                    // Get the string index from the operand's integer literal field
                    int iStringIndex = pOpList[iCurrOpIndex].iIntLiteral;
                    // Allocate a new string to hold a copy of the one in the table
                    char *pstrStringCopy = (char *) malloc(strlen(ppstrStringTable[iStringIndex]) + 1);

                    // Make a copy of the string
                    strcpy(pstrStringCopy, ppstrStringTable[iStringIndex]);
                    // Save the string pointer in the operand list
                    pOpList[iCurrOpIndex].pstrStringLiteral = pstrStringCopy;
                }
            }
        }

        // Free the original strings
        for (iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++iCurrStringIndex)
            free(ppstrStringTable[iCurrStringIndex]);
        // ---- Free the string table itself
        free(ppstrStringTable);
    }

    // ---- The Function Table ---------------------------------------------------------

    int iFuncTableSize;
    fread(&iFuncTableSize, 4, 1, pScriptFile);
    // Allocate the table
    g_Script.pFuncTable = (Func *) malloc(iFuncTableSize * sizeof(Func));


    for (int iCurrFuncIndex = 0; iCurrFuncIndex < iFuncTableSize; ++iCurrFuncIndex) {
        // Read the entry point (4 bytes)
        int iEntryPoint;
        fread(&iEntryPoint, 4, 1, pScriptFile);
        // Read the parameter count (1 byte)
        int iParamCount = 0;
        fread(&iParamCount, 1, 1, pScriptFile);
        // Read the local data size (4 bytes)
        int iLocalDataSize;
        fread(&iLocalDataSize, 4, 1, pScriptFile);
        // Calculate the stack size
        int iStackFrameSize = iParamCount + 1 + iLocalDataSize;
        // Write everything to the function table
        g_Script.pFuncTable[iCurrFuncIndex].iEntryPoint = iEntryPoint;
        g_Script.pFuncTable[iCurrFuncIndex].iParamCount = iParamCount;
        g_Script.pFuncTable[iCurrFuncIndex].iLocalDataSize = iLocalDataSize;
        g_Script.pFuncTable[iCurrFuncIndex].iStackFrameSize = iStackFrameSize;
    }

    // ---- The Host API Call Table ------------------------------------------------------

    // Read the host API call count
    fread(&g_Script.HostAPICallTable.iSize, 4, 1, pScriptFile);
    // Allocate space for the table
    g_Script.HostAPICallTable.ppstrCalls = (char **) malloc(g_Script.HostAPICallTable.iSize * sizeof(char *));

    // Read each host API call
    for (int iCurrCallIndex = 0; iCurrCallIndex < g_Script.HostAPICallTable.iSize; ++iCurrCallIndex) {
        // Read the host API call string size (1 byte)
        int iCallLength = 0;
        fread(&iCallLength, 1, 1, pScriptFile);
        // Allocate space for the string plus the null terminator in a temporary pointer
        char *pstrCurrCall = (char *) malloc(iCallLength + 1);
        // Read the host API call string data and append the null terminator
        fread(pstrCurrCall, iCallLength, 1, pScriptFile);
        pstrCurrCall[iCallLength] = '\0';
        // Assign the temporary pointer to the table
        g_Script.HostAPICallTable.ppstrCalls[iCurrCallIndex] = pstrCurrCall;
    }

    // ---- Close the input file
    fclose(pScriptFile);

    printf("%s loaded successfully!\n", pstrFilename);
    printf("\n");
    printf("  Format Version: %d.%d\n", iMajorVersion, iMinorVersion);
    printf("      Stack Size: %d\n", g_Script.Stack.iSize);
    printf("Global Data Size: %d\n", g_Script.iGlobalDataSize);
    printf("       Functions: %d\n", iFuncTableSize);
    printf("_Main () Present: ");
    if (g_Script.iIsMainFuncPresent)
        printf("Yes (Index %d)", g_Script.iMainFuncIndex);
    else
        printf("No");
    printf("\n");
    printf("  Host API Calls: %d\n", g_Script.HostAPICallTable.iSize);
    printf("    Instructions: %d\n", g_Script.InstrStream.iSize);
    printf(" String Literals: %d\n", iStringTableSize);
    printf("\n");

    return LOAD_OK;
}

void ResetScript() {
    // Get _Main ()'s function index in case we need it
    int iMainFuncIndex = g_Script.iMainFuncIndex;
    // If the function table is present, set the entry point
    if (g_Script.pFuncTable) {
        // If _Main () is present, read _Main ()'s index of the function table to get its
        // entry point
        if (g_Script.iIsMainFuncPresent) {
            g_Script.InstrStream.iCurrInstr = g_Script.pFuncTable[iMainFuncIndex].iEntryPoint;
        }
    }
    // Clear the stack
    g_Script.Stack.iTopIndex = 0;
    g_Script.Stack.iFrameIndex = 0;
    // Set the entire stack to null
    for (int iCurrElmntIndex = 0; iCurrElmntIndex < g_Script.Stack.iSize; ++iCurrElmntIndex)
        g_Script.Stack.pElmnts[iCurrElmntIndex].iType = OP_TYPE_NULL;
    // Unpause the script
    g_Script.iIsPaused = false;
    // Allocate space for the globals
    PushFrame(g_Script.iGlobalDataSize);
    // If _Main () is present, push it's stack frame (plus one extra stack element to
    // compensate for the function index that usually sits on top of stack frames and
    // causes indices to start from -2)
    PushFrame(g_Script.pFuncTable[iMainFuncIndex].iStackFrameSize + 1);
}

int RunScript() {
    int iExitExecLoop = FALSE;
    int iExitCode;

    while (TRUE) {

        int iCurrInstr = g_Script.InstrStream.iCurrInstr;
        // Get the current opcode
        int iOpcode = g_Script.InstrStream.pInstrs[iCurrInstr].iOpCode;
        if (PRINT_INSTR) {
            printf("\t");
            printf("%d", iCurrInstr);
            printf("\t");
            printf(" %s ", ppstrMnemonics[iOpcode]);
        }
        switch (iOpcode) {
            case INSTR_MOV:
                // Arithmetic Operations
            case INSTR_ADD:
            case INSTR_SUB:
            case INSTR_MUL:
            case INSTR_DIV:
            case INSTR_MOD:
            case INSTR_EXP:
                // Bitwise Operations
            case INSTR_AND:
            case INSTR_OR:
            case INSTR_XOR:
            case INSTR_SHL:
            case INSTR_SHR: {
                Value Dest = ResolveOpValue(0);
                Value Source = ResolveOpValue(1);
                switch (iOpcode) {
                    case INSTR_MOV:
                        if (ResolveOpPntr(0) == ResolveOpPntr(1))
                            break;
                        CopyValue(&Dest, Source);
                        break;

                    case INSTR_ADD:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral += ResolveOpAsInt(1);
                        else
                            Dest.fFloatLiteral += ResolveOpAsFloat(1);
                        break;
                        // Subtract
                    case INSTR_SUB:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral -= ResolveOpAsInt(1);
                        else
                            Dest.fFloatLiteral -= ResolveOpAsFloat(1);
                        break;
                        // Multiply
                    case INSTR_MUL:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral *= ResolveOpAsInt(1);
                        else
                            Dest.fFloatLiteral *= ResolveOpAsFloat(1);
                        break;
                        // Divide
                    case INSTR_DIV:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral /= ResolveOpAsInt(1);
                        else
                            Dest.fFloatLiteral /= ResolveOpAsFloat(1);
                        break;
                        // Modulus
                    case INSTR_MOD:
                        // Remember, Mod works with integers only
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral %= ResolveOpAsInt(1);
                        break;
                        // Exponentiate
                    case INSTR_EXP:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral = (int) pow(Dest.iIntLiteral, ResolveOpAsInt(1));
                        else
                            Dest.fFloatLiteral = (float) pow(Dest.fFloatLiteral, ResolveOpAsFloat(1));
                        break;
                        // The bitwise instructions only work with integers. They do nothing
                        // when the destination data type is anything else.
                        // And
                    case INSTR_AND:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral &= ResolveOpAsInt(1);
                        break;
                        // Or
                    case INSTR_OR:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral |= ResolveOpAsInt(1);
                        break;
                        // Exclusive Or
                    case INSTR_XOR:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral ^= ResolveOpAsInt(1);
                        break;
                        // Shift Left
                    case INSTR_SHL:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral <<= ResolveOpAsInt(1);
                        break;
                        // Shift Right
                    case INSTR_SHR:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral >>= ResolveOpAsInt(1);
                        break;
                    default:;
                }
                Value *_tmp = ResolveOpPntr(0);
                if (_tmp)
                    *_tmp = Dest;

                if (PRINT_INSTR) {
                    PrintOpIndir(0);
                    printf(", ");
                    PrintOpValue(1);
                }
                break;
            }

            case INSTR_NEG:
            case INSTR_NOT:
            case INSTR_INC:
            case INSTR_DEC:
            case INSTR_SQRT: {
                int iDestStoreType = GetOpType(0);
                Value Dest = ResolveOpValue(0);

                switch (iOpcode) {
                    case INSTR_NEG:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral = -Dest.iIntLiteral;
                        else
                            Dest.fFloatLiteral = -Dest.fFloatLiteral;
                        break;

                        // Not
                    case INSTR_NOT:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral = ~Dest.iIntLiteral;
                        break;
                        // Increment
                    case INSTR_INC:
                        if (Dest.iType == OP_TYPE_INT)
                            ++Dest.iIntLiteral;
                        else
                            ++Dest.fFloatLiteral;
                        break;
                        // Decrement
                    case INSTR_DEC:
                        if (Dest.iType == OP_TYPE_INT)
                            --Dest.iIntLiteral;
                        else
                            --Dest.fFloatLiteral;
                        break;
                    case INSTR_SQRT:
                        if (Dest.iType == OP_TYPE_INT)
                            Dest.iIntLiteral = Dest.iIntLiteral * Dest.iIntLiteral;
                        else
                            Dest.fFloatLiteral = Dest.fFloatLiteral * Dest.fFloatLiteral;
                    default:;
                }
                *ResolveOpPntr(0) = Dest;
                if (PRINT_INSTR) {
                    PrintOpIndir(0);
                }
                break;
            }

            case INSTR_CONCAT: {
                // Get a local copy of the destination operand (operand index 0)
                Value Dest = ResolveOpValue(0);
                // Get a local copy of the source string (operand index 1)
                char *pstrSourceString = ResolveOpAsString(1);
                // If the destination isn't a string, do nothing
                if (Dest.iType != OP_TYPE_STRING)
                    break;
                // Determine the length of the new string and allocate space for it (with a
                // null terminator)
                int iNewStringLength = strlen(Dest.pstrStringLiteral) + strlen(pstrSourceString);
                char *pstrNewString = (char *) malloc(iNewStringLength + 1);
                // Copy the old string to the new one
                strcpy(pstrNewString, Dest.pstrStringLiteral);
                // Concatenate the destination with the source
                strcat(pstrNewString, pstrSourceString);
                // Free the existing string in the destination structure and replace it

                // with the new string
                free(Dest.pstrStringLiteral);
                //!----------------------maybe bug : no free pstrSourceString----------------------!
                if (ResolveOpValue(1).iType != OP_TYPE_STRING) {
                    free(pstrSourceString);
                }
                //----------------------fix bug----------------------------------//
                point = pstrSourceString;


                Dest.pstrStringLiteral = pstrNewString;
                // Copy the concatenated string pointer to its destination
                *ResolveOpPntr(0) = Dest;
                // Print out the operands
                if (PRINT_INSTR) {
                    PrintOpIndir(0);
                    printf(", ");
                    PrintOpValue(1);
                }
                break;
            }

            case INSTR_GETCHAR: {
                // Get a local copy of the destination operand (operand index 0)
                Value Dest = ResolveOpValue(0);
                // Get a local copy of the source string (operand index 1)
                char *pstrSourceString = ResolveOpAsString(1);
                // Find out whether or not the destination is already a string
                char *pstrNewString;
                if (Dest.iType == OP_TYPE_STRING) {
                    // If it is, we can use it's existing string buffer as long as it's at
                    // least 1 character
                    if (strlen(Dest.pstrStringLiteral) >= 1) {
                        pstrNewString = Dest.pstrStringLiteral;
                    } else {
                        free(Dest.pstrStringLiteral);
                        pstrNewString = (char *) malloc(2);
                    }
                } else {
                    // Otherwise allocate a new string and set the type
                    pstrNewString = (char *) malloc(2);
                    Dest.iType = OP_TYPE_STRING;
                }
                // Get the index of the character (operand index 2)
                int iSourceIndex = ResolveOpAsInt(2);
                // Copy the character and append a null-terminator
                pstrNewString[0] = pstrSourceString[iSourceIndex];
                pstrNewString[1] = '\0';
                // Set the string pointer in the destination Value structure
                Dest.pstrStringLiteral = pstrNewString;
                // Copy the concatenated string pointer to its destination
                *ResolveOpPntr(0) = Dest;
                // Print out the operands
                if (PRINT_INSTR) {
                    PrintOpIndir(0);
                    printf(", ");
                    PrintOpValue(1);
                    printf(", ");
                    PrintOpValue(2);
                }
                break;

            }

            case INSTR_SETCHAR: {
                // Get the destination index (operand index 2)
                int iDestIndex = ResolveOpAsInt(2);
                // If the destination isn't a string, do nothing
                if (ResolveOpType(0) != OP_TYPE_STRING)
                    break;
                // Get the source character (operand index 2)
                char *pstrSourceString = ResolveOpAsString(1);
                // Set the specified character in the destination (operand index 0)
                ResolveOpPntr(0)->pstrStringLiteral[iDestIndex] = pstrSourceString[0];
                // Print out the operands
                if (PRINT_INSTR) {
                    PrintOpIndir(0);
                    printf(", ");
                    PrintOpValue(1);
                    printf(", ");
                    PrintOpValue(2);
                }
                break;
            }

            case INSTR_JMP: {
                // Get the index of the target instruction (opcode index 0)
                int iTargetIndex = ResolveOpAsInstrIndex(0);
                // Move the instruction pointer to the target
                g_Script.InstrStream.iCurrInstr = iTargetIndex;
                // Print out the target index
                PrintOpValue(0);
                break;
            }

            case INSTR_JE:
            case INSTR_JNE:
            case INSTR_JG:
            case INSTR_JL:
            case INSTR_JGE:
            case INSTR_JLE: {
                // Get the two operands
                Value Op0 = ResolveOpValue(0);
                Value Op1 = ResolveOpValue(1);
                // Get the index of the target instruction (opcode index 2)
                int iTargetIndex = ResolveOpAsInstrIndex(2);
                // Perform the specified comparison and jump if it evaluates to true
                int iJump = FALSE;
                switch (iOpcode) {
                    // Jump if Equal
                    case INSTR_JE: {
                        switch (Op0.iType) {
                            case OP_TYPE_INT:
                                if (Op0.iIntLiteral == Op1.iIntLiteral)
                                    iJump = TRUE;
                                break;
                            case OP_TYPE_FLOAT:
                                //bug
                                if (fabs(Op0.fFloatLiteral - Op1.fFloatLiteral) <= ESP)
                                    iJump = TRUE;
                                //bug fix
                                break;
                            case OP_TYPE_STRING:
                                if (strcmp(Op0.pstrStringLiteral, Op1.pstrStringLiteral) == 0)
                                    iJump = TRUE;
                                break;
                        }
                        break;
                    }
                        // Jump if Not Equal
                    case INSTR_JNE: {
                        switch (Op0.iType) {
                            case OP_TYPE_INT:
                                if (Op0.iIntLiteral != Op1.iIntLiteral)
                                    iJump = TRUE;
                                break;
                            case OP_TYPE_FLOAT:
                                if (Op0.fFloatLiteral != Op1.fFloatLiteral)
                                    iJump = TRUE;
                                break;
                            case OP_TYPE_STRING:
                                if (strcmp(Op0.pstrStringLiteral, Op1.pstrStringLiteral) != 0)
                                    iJump = TRUE;
                                break;
                        }

                        break;

                    }

                        // Jump if Greater

                    case INSTR_JG:
                        if (Op0.iType == OP_TYPE_INT) {
                            if (Op0.iIntLiteral > Op1.iIntLiteral)
                                iJump = TRUE;
                        } else {
                            if (Op0.fFloatLiteral > Op1.fFloatLiteral)
                                iJump = TRUE;
                        }
                        break;
                        // Jump if Less
                    case INSTR_JL:
                        if (Op0.iType == OP_TYPE_INT) {
                            if (Op0.iIntLiteral < Op1.iIntLiteral)
                                iJump = TRUE;
                        } else {
                            if (Op0.fFloatLiteral < Op1.fFloatLiteral)
                                iJump = TRUE;
                        }
                        break;
                        // Jump if Greater or Equal
                    case INSTR_JGE:
                        if (Op0.iType == OP_TYPE_INT) {
                            if (Op0.iIntLiteral >= Op1.iIntLiteral)
                                iJump = TRUE;
                        } else {
                            if (Op0.fFloatLiteral >= Op1.fFloatLiteral)
                                iJump = TRUE;
                        }
                        break;
                        // Jump if Less or Equal
                    case INSTR_JLE:
                        if (Op0.iType == OP_TYPE_INT) {
                            if (Op0.iIntLiteral <= Op1.iIntLiteral)
                                iJump = TRUE;
                        } else {
                            if (Op0.fFloatLiteral <= Op1.fFloatLiteral)
                                iJump = TRUE;
                        }
                        break;
                }
                // Print out the operands
                if (PRINT_INSTR) {
                    PrintOpValue(0);
                    printf(", ");
                    PrintOpValue(1);
                    printf(", ");
                    PrintOpValue(2);
                    printf(" ");
                }
                // If the comparison evaluated to TRUE, make the jump
                if (iJump) {
                    g_Script.InstrStream.iCurrInstr = iTargetIndex;
                    if (PRINT_INSTR)
                        printf("(True)");
                } else {
                    if (PRINT_INSTR)
                        printf("(False)");

                }
                break;
            }

                // ---- The Stack Interface
            case INSTR_PUSH: {
                // Get a local copy of the source operand (operand index 0)

                //bug about string
                // Push the value onto the stack
                Push(ResolveOpValue(0));
                //no bug

                // Print the source
                if (PRINT_INSTR)
                    PrintOpValue(0);
                break;
            }
            case INSTR_POP: {
                // Pop the top of the stack into the destination
                //maybe bug
                *ResolveOpPntr(0) = Pop();
                // no bug
                // Print the destination
                if (PRINT_INSTR)
                    PrintOpIndir(0);
                break;
            }
            case INSTR_CALL: {

                // Get a local copy of the function index (operand index 0) and function
                // structure
                int iFuncIndex = ResolveOpAsFuncIndex(0);
                int iFrameIndex = g_Script.Stack.iFrameIndex;
                Func Dest = GetFunc(iFuncIndex);
                // Push the return address which is the instruction just beyond the current
                // one
                Value ReturnAddr;
                ReturnAddr.iType = OP_TYPE_INSTR_INDEX;
                ReturnAddr.iInstrIndex = g_Script.InstrStream.iCurrInstr + 1;

                Push(ReturnAddr);
                // Push the stack frame + 1 (the extra space is for the function index
                // we'll put on the stack after it
                PushFrame(Dest.iLocalDataSize + 1);
                // Write the function index to the top of the stack
                Value FuncIndex;
                FuncIndex.iType = OP_TYPE_NULL;
                FuncIndex.iFuncIndex = iFuncIndex;
                FuncIndex.iOffsetIndex = iFrameIndex;
                SetStackValue(g_Script.Stack.iTopIndex - 1, FuncIndex);
                // Make the jump to the function's entry point
                g_Script.InstrStream.iCurrInstr = Dest.iEntryPoint;
                if (PRINT_INSTR) {
                    printf("$$[%d]$$", g_Script.InstrStream.iCurrInstr);
                    // Print out some information
                    printf("%d ( Entry Point: %d, Frame Size: %d )", iFuncIndex, Dest.iEntryPoint,
                           Dest.iStackFrameSize);
                }
                break;
            }
            case INSTR_RET: {
                // Get the current function index off the top of the stack and use it to get
                // the corresponding function structure
                Value FuncIndex = Pop();
                Func CurrFunc = GetFunc(FuncIndex.iFuncIndex);
                int iFrameIndex = FuncIndex.iOffsetIndex;
                // Read the return address structure from the stack, which is stored one
                // index below the local data
                Value ReturnAddr = GetStackValue(g_Script.Stack.iTopIndex - (CurrFunc.iLocalDataSize + 1));
                // Pop the stack frame along with the return address
                PopFrame(CurrFunc.iStackFrameSize);
                // Restore the previous frame index
                g_Script.Stack.iFrameIndex = iFrameIndex;
                // Make the jump to the return address
                g_Script.InstrStream.iCurrInstr = ReturnAddr.iInstrIndex;
                // Print the return address
                if (PRINT_INSTR)
                    printf("%d", ReturnAddr.iInstrIndex);
                break;
            }
            case INSTR_CALLHOST: {
                // CallHost is not implemented in this prototype, so just print out the
                // name of the function
                int i;
                PrintOpValue(0);

                for (i = 0; i < g_Script.HostAPICallTable.iFuncNum; i++) {
                    if (strcmp(ResolveOpAsHostAPICall(0), g_Script.HostAPICallTable.ptrHostFuncTable[i].ptrFuncName) ==
                        0) {
                        g_Script.HostAPICallTable.ptrHostFuncTable[i].fHostfunc();
                    }

                }


                break;
            }
            case INSTR_PAUSE: {
                // Get the pause duration
                int iPauseDuration = ResolveOpAsInt(0);
                // Determine the ending pause time
                sleep(iPauseDuration);
                // Print the pause duration
                if (PRINT_INSTR)
                    PrintOpValue(0);
                break;
            }
            case INSTR_EXIT: {

                iExitCode = ResolveOpValue(0).iIntLiteral;
                // Break the execution cycle
                iExitExecLoop = TRUE;
                // Print the exit code
                if (PRINT_INSTR)
                    PrintOpValue(0);
                break;


            }
                // Resolve operand zero to find the exit code

            default:
                if (PRINT_INSTR)
                    printf("UNKNOW OPCODE : %d\n exiting...", iOpcode);
                iExitExecLoop = TRUE;
                iExitCode = 1;


        }
        if (PRINT_INSTR)
            printf("\n");

        // Has IP changed during the instruction's execution?
        if (iCurrInstr == g_Script.InstrStream.iCurrInstr)
            // No, so increment it
            ++g_Script.InstrStream.iCurrInstr;

        if (iExitExecLoop)
            break;

    }
    return iExitCode;
}


int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("need a file!\n");
        exit(0);
    }
    int i = 0;

    Init();
    if ((i = LoadScript(argv[1])) != LOAD_OK)
        printf("wrong! id:%d\n ", i);
    ResetScript();
    RunScript();
    ShutDown();

    return 0;
}

