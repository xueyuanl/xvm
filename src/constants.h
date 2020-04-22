//
// Created by hsuehyuan li on 2020-04-12.
//

#ifndef XVM_CONSTANTS_H
#define XVM_CONSTANTS_H

#define MAX_COERCION_STRING_SIZE    64

#define OP_TYPE_NULL                -1
#define OP_TYPE_INT                 0           // Integer literal value
#define OP_TYPE_FLOAT               1           // Floating-point literal value
#define OP_TYPE_STRING              2           // String literal value
#define OP_TYPE_ABS_STACK_INDEX     3           // Absolute array index
#define OP_TYPE_REL_STACK_INDEX     4           // Relative array index
#define OP_TYPE_INSTR_INDEX         5           // Instruction index
#define OP_TYPE_FUNC_INDEX          6           // Function index
#define OP_TYPE_HOST_API_CALL_INDEX 7           // Host API call index
#define OP_TYPE_REG                 8           // Register

// ---- LoadScript () Error Codes ---------------------------------------------------------

#define LOAD_OK						0			// Load successful
#define LOAD_ERROR_FILE_IO  	    1			// File I/O error (most likely a file
// not found error
#define LOAD_ERROR_INVALID_XSE		2			// Invalid .XSE structure
#define LOAD_ERROR_UNSUPPORTED_VERS	3			// The format version is unsupported


#define XSE_ID_STRING               "XSE0"      // Used to validate an .XSE executable

// ---- Stack -----------------------------------------------------------------------------

#define DEF_STACK_SIZE			    1024	    // The default stack size

#endif //XVM_CONSTANTS_H
