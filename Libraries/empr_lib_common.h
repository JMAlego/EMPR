#ifndef EMPR_LIB_COMMON_H
#define EMPR_LIB_COMMON_H

//Make sure we have all standard int types
#include <stdint.h>
#include "lpc_types.h"

//You've got to have PI
#define PI 3.1415926535

#ifndef __size_t &&
//Fix issue with includes in vscode
typedef uint32_t size_t;
#endif

#endif
