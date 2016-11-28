//Main.h
#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/types.h"
#include "dynamic_libs/os_functions.h"

/* Main */
#ifdef __cplusplus
extern "C" {
#endif

//! C wrapper for our C++ functions
int Menu_Main(void);
void uhs_exploit_init(void);
int uhs_write32(int, int);

#ifdef __cplusplus
}
#endif

#endif
