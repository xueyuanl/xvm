//
// Created by pat on 4/8/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "structure.h"

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

