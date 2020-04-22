//
// Created by mystic on 2020/4/22.
//

#ifndef XVM_STACK_H
#define XVM_STACK_H

#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "constants.h"

#define ResolveStackIndex( iIndex ) ( iIndex < 0 ? iIndex += g_Script.Stack.iFrameIndex : iIndex )

#endif //XVM_STACK_H
