/**
 * @file fault.c
 *
 * This file implements the screen that may be viewed when the game crashes.
 * This is the second known version of the crash screen, an evolved version from OoT's.
 *
 * When the game crashes, a red bar will be drawn to the top-left of the screen, indicating that the
 * crash screen is available for use. Once this bar appears, it is possible to open the crash screen
 * with the following button combination:
 *
 * (DPad-Left & L & R & C-Right) & Start
 *
 * When entering this button combination, buttons that are &'d together must all be pressed together.
 *
 * "Clients" may be registered with the crash screen to extend its functionality. There are
 * two kinds of client, "Client" and "AddressConverterClient". Clients contribute one or
 * more pages to the crash debugger, while Address Converter Clients allow the crash screen to look up
 * the virtual addresses of dynamically allocated overlays.
 *
 * The crash screen has multiple pages:
 *  - Thread Context
 *      This page shows information about the thread on which the program crashed. It displays
 *      the cause of the crash, state of general-purpose registers, state of floating-point registers
 *      and the floating-point status register. If a floating-point exception caused the crash, it will
 *      be displayed next to the floating-point status register.
 *  - Stack Trace
 *      This page displays a full backtrace from the crashing function back to the start of the thread. It
 *      displays the Program Counter for each function and, if applicable, the Virtual Program Counter
 *      for relocated functions in overlays.
 *  - Client Pages
 *      After the stack trace page, currently registered clients are processed and their pages are displayed.
 *  - Memory Dump
 *      This page implements a scrollable memory dump.
 *  - End Screen
 *      This page informs you that there are no more pages to display.
 *
 * To navigate the pages, START and A may be used to advance to the next page, and L toggles whether to
 * automatically scroll to the next page after some time has passed.
 * DPad-Up may be pressed to enable sending fault pages over osSyncPrintf as well as displaying them on-screen.
 * DPad-Down disables sending fault pages over osSyncPrintf.
 */
#include "fault.h"
#include "fault_internal.h"

#include "libc64/sleep.h"
#include "libc64/sprintf.h"
#include "PR/osint.h"

#include "controller.h"
#include "macros.h"
#include "main.h"
#include "vt.h"
#include "libu64/stackcheck.h"
#include "z64thread.h"
#include "libc/string.h"
#include "chaos_fuckery.h"
#include "z64game.h"

FaultMgr* sFaultInstance;
f32 sFaultTimeTotal; // read but not set anywhere
extern struct ChaosContext  gChaosContext;
extern const char *         gChaosCodeNames[];
extern struct GameState *   gCurrentGameState;

#define LAST_LEFT_SUBPAGE  1
#define LAST_RIGHT_SUBPAGE 2

// data
const char* sCpuExceptions[] = {
    "Interrupt",
    "TLB modification",
    "TLB exception on load",
    "TLB exception on store",
    "Address error on load",
    "Address error on store",
    "Bus error on inst.",
    "Bus error on data",
    "System call exception",
    "Breakpoint exception",
    "Reserved instruction",
    "Coprocessor unusable",
    "Arithmetic overflow",
    "Trap exception",
    "Virtual coherency on inst.",
    "Floating point exception",
    "Watchpoint exception",
    "Virtual coherency on data",
};

const char* sFpuExceptions[] = {
    "Unimplemented operation", "Invalid operation", "Division by zero", "Overflow", "Underflow", "Inexact operation",
};

const char *sRegNames[] = {
    "z",
    "at",
    "v0",
    "v1",
    "a0",
    "a1",
    "a2",
    "a3",
    "t0",
    "t1",
    "t2",
    "t3",
    "t4",
    "t5",
    "t6",
    "t7",
    "s0",
    "s1",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "t8",
    "t9",
    "k0",
    "k1",
    "gp",
    "sp",
    "fp",
    "ra"
};

enum FAULT_INST_FORMAT_STRS
{
    FAULT_INST_FORMAT_STR_RT_OFFSET_BASE,
    FAULT_INST_FORMAT_STR_RT_RS_IMMEDIATE,
    FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE,
    FAULT_INST_FORMAT_STR_RD_RS_RT,
    FAULT_INST_FORMAT_STR_OP_OFFSET_BASE,
    FAULT_INST_FORMAT_STR_TARGET,
    FAULT_INST_FORMAT_STR_RS_RT_OFFSET,
    FAULT_INST_FORMAT_STR_RS_OFFSET,
    FAULT_INST_FORMAT_STR_RD_RT_SA,
    FAULT_INST_FORMAT_STR_RD_RT_RS,
    FAULT_INST_FORMAT_STR_RS_RT,
    FAULT_INST_FORMAT_STR_RD,
    FAULT_INST_FORMAT_STR_RS,
    FAULT_INST_FORMAT_STR_RT_IMMEDIATE,
    FAULT_INST_FORMAT_STR_RS_IMMEDIATE,
    FAULT_INST_FORMAT_STR_RS_RD,
    FAULT_INST_FORMAT_STR_NONE
};

const char *sInstructionFormatStrs[] = {
    /* [FAULT_INST_FORMAT_STR_RT_OFFSET_BASE] = */  "%-05s %s,%s%04x(%s)",
    /* [FAULT_INST_FORMAT_STR_RT_RS_IMMEDIATE] = */ "%-05s %s, %s, %04x",
    /* [FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE] = */"%-05s %s, %s,%s%04x",
    /* [FAULT_INST_FORMAT_STR_RD_RS_RT] = */        "%-05s %s, %s, %s",
    /* [FAULT_INST_FORMAT_STR_OP_OFFSET_BASE] = */  "%-05s %02x,%s%04x(%s)",
    /* [FAULT_INST_FORMAT_STR_TARGET] = */          "%-05s %08x",
    /* [FAULT_INST_FORMAT_STR_RS_RT_OFFSET] = */    "%-05s %s, %s,%s%04x",
    /* [FAULT_INST_FORMAT_STR_RS_OFFSET] = */       "%-05s %s,%s%04x",
    /* [FAULT_INST_FORMAT_STR_RD_RT_SA] = */        "%-05s %s, %s, %s",
    /* [FAULT_INST_FORMAT_STR_RD_RT_RS] = */        "%-05s %s, %s, %s",
    /* [FAULT_INST_FORMAT_STR_RS_RT] = */           "%-05s %s, %s",
    /* [FAULT_INST_FORMAT_STR_RD] = */              "%-05s %s",
    /* [FAULT_INST_FORMAT_STR_RS] = */              "%-05s %s",
    /* [FAULT_INST_FORMAT_STR_RT_IMMEDIATE] = */    "%-05s %s, %04x",
    /* [FAULT_INST_FORMAT_STR_RS_IMMEDIATE] = */    "%-05s %s, %04x",
    /* [FAULT_INST_FORMAT_STR_RS_RD] = */           "%-05s %s, %s" 
};

const char *sOpcodeStrs[] = {
    /* [FAULT_INST_OPCODE_SPECIAL] =  */    "SPECIAL",
    /* [FAULT_INST_OPCODE_REGIMM] =  */     "REGIMM",
    /* [FAULT_INST_OPCODE_J] =  */          "J",
    /* [FAULT_INST_OPCODE_JAL] =  */        "JAL",
    /* [FAULT_INST_OPCODE_BEQ] =  */        "BEQ",
    /* [FAULT_INST_OPCODE_BNE] =  */        "BNE",
    /* [FAULT_INST_OPCODE_BLEZ] =  */       "BLEZ",
    /* [FAULT_INST_OPCODE_BGTZ] =  */       "BGTZ",
    /* [FAULT_INST_OPCODE_ADDI] =  */       "ADDI",
    /* [FAULT_INST_OPCODE_ADDIU] =  */      "ADDIU",
    /* [FAULT_INST_OPCODE_SLTI] =  */       "SLTI",
    /* [FAULT_INST_OPCODE_SLTIU] =  */      "SLTIU",
    /* [FAULT_INST_OPCODE_ANDI] =  */       "ANDI",
    /* [FAULT_INST_OPCODE_ORI] =  */        "ORI",
    /* [FAULT_INST_OPCODE_XORI] =  */       "XORI",
    /* [FAULT_INST_OPCODE_LUI] =  */        "LUI",
    /* [FAULT_INST_OPCODE_COP0] =  */       "COP0",
    /* [FAULT_INST_OPCODE_COP1] =  */       "COP1",
    /* [FAULT_INST_OPCODE_COP2] =  */       "COP2",
    /* [FAULT_INST_OPCODE_RESERVED0] =  */  "RESERVED0",
    /* [FAULT_INST_OPCODE_BEQL] =  */       "BEQL",
    /* [FAULT_INST_OPCODE_BNEL] =  */       "BNEL",
    /* [FAULT_INST_OPCODE_BLEZL] =  */      "BLEZL",
    /* [FAULT_INST_OPCODE_BGTZL] =  */      "BGTZL",
    /* [FAULT_INST_OPCODE_DADDI] =  */      "DADDI",
    /* [FAULT_INST_OPCODE_DADDIU] =  */     "DADDIU",
    /* [FAULT_INST_OPCODE_LDL] =  */        "LDL",
    /* [FAULT_INST_OPCODE_LDR] =  */        "LDR",
    /* [FAULT_INST_OPCODE_RESERVED1] =  */  "RESERVED1",
    /* [FAULT_INST_OPCODE_RESERVED2] =  */  "RESERVED2",
    /* [FAULT_INST_OPCODE_RESERVED3] =  */  "RESERVED3",
    /* [FAULT_INST_OPCODE_RESERVED4] =  */  "RESERVED4",
    /* [FAULT_INST_OPCODE_LB] =  */         "LB",
    /* [FAULT_INST_OPCODE_LH] =  */         "LH",
    /* [FAULT_INST_OPCODE_LWL] =  */        "LWL",
    /* [FAULT_INST_OPCODE_LW] =  */         "LW",
    /* [FAULT_INST_OPCODE_LBU] =  */        "LBU",
    /* [FAULT_INST_OPCODE_LHU] =  */        "LHU",
    /* [FAULT_INST_OPCODE_LWR] =  */        "LWR",
    /* [FAULT_INST_OPCODE_LWU] =  */        "LWU",
    /* [FAULT_INST_OPCODE_SB] =  */         "SB",
    /* [FAULT_INST_OPCODE_SH] =  */         "SH",
    /* [FAULT_INST_OPCODE_SWL] =  */        "SWL",
    /* [FAULT_INST_OPCODE_SW] =  */         "SW",
    /* [FAULT_INST_OPCODE_SDL] =  */        "SDL",
    /* [FAULT_INST_OPCODE_SDR] =  */        "SDR",
    /* [FAULT_INST_OPCODE_SWR] =  */        "SWR",
    /* [FAULT_INST_OPCODE_CACHE] =  */      "CACHE",
    /* [FAULT_INST_OPCODE_LL] =  */         "LL",
    /* [FAULT_INST_OPCODE_LWC1] =  */       "LWC1",
    /* [FAULT_INST_OPCODE_LWC2] =  */       "LWC2",
    /* [FAULT_INST_OPCODE_RESERVED5] =  */  "RESERVED5",
    /* [FAULT_INST_OPCODE_LLD] =  */        "LLD",
    /* [FAULT_INST_OPCODE_LDC1] =  */       "LDC1",
    /* [FAULT_INST_OPCODE_LDC2] =  */       "LDC2",
    /* [FAULT_INST_OPCODE_LD] =  */         "LD",
    /* [FAULT_INST_OPCODE_SC] =  */         "SC",
    /* [FAULT_INST_OPCODE_SWC1] =  */       "SWC1",
    /* [FAULT_INST_OPCODE_SWC2] =  */       "SWC2",
    /* [FAULT_INST_OPCODE_RESERVED6] =  */  "RESERVED6",
    /* [FAULT_INST_OPCODE_SCD] =  */        "SCD",
    /* [FAULT_INST_OPCODE_SDC1] =  */       "SDC1",
    /* [FAULT_INST_OPCODE_SDC2] =  */       "SDC2",
    /* [FAULT_INST_OPCODE_SD] =  */         "SD"
};

// const char *sOpcodeFormatStrs[] = {
//     /* [FAULT_INST_CLASS_I] */ "r%02d, r%02d, %04x",
//     /* [FAULT_INST_CLASS_J] */,
//     /* [FAULT_INST_CLASS_R] */,
// };

struct FaultInstInfo sOpcodeInfos[] = {
    /* [FAULT_INST_OPCODE_SPECIAL] =  */    FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_REGIMM] =  */     FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_J] =  */          FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_TARGET),
    /* [FAULT_INST_OPCODE_JAL] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_TARGET),
    /* [FAULT_INST_OPCODE_BEQ] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT_OFFSET),
    /* [FAULT_INST_OPCODE_BNE] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT_OFFSET),
    /* [FAULT_INST_OPCODE_BLEZ] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_OFFSET),
    /* [FAULT_INST_OPCODE_BGTZ] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_OFFSET),
    /* [FAULT_INST_OPCODE_ADDI] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE),
    /* [FAULT_INST_OPCODE_ADDIU] =  */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE),
    /* [FAULT_INST_OPCODE_SLTI] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE),
    /* [FAULT_INST_OPCODE_SLTIU] =  */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE),
    /* [FAULT_INST_OPCODE_ANDI] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_RS_IMMEDIATE),
    /* [FAULT_INST_OPCODE_ORI] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_RS_IMMEDIATE),
    /* [FAULT_INST_OPCODE_XORI] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_RS_IMMEDIATE),
    /* [FAULT_INST_OPCODE_LUI] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_IMMEDIATE),
    /* [FAULT_INST_OPCODE_COP0] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_COP1] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_COP2] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_RESERVED0] =  */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_BEQL] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT_OFFSET),
    /* [FAULT_INST_OPCODE_BNEL] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT_OFFSET),
    /* [FAULT_INST_OPCODE_BLEZL] =  */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_OFFSET),
    /* [FAULT_INST_OPCODE_BGTZL] =  */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_OFFSET),
    /* [FAULT_INST_OPCODE_DADDI] =  */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE),
    /* [FAULT_INST_OPCODE_DADDIU] =  */     FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE),
    /* [FAULT_INST_OPCODE_LDL] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LDR] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_RESERVED1] =  */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_RESERVED2] =  */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_RESERVED3] =  */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_RESERVED4] =  */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_LB] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LH] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LWL] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LW] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LBU] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LHU] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LWR] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LWU] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SB] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SH] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SWL] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SW] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SDL] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SDR] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SWR] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_CACHE] =  */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_OP_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LL] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LWC1] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LWC2] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_RESERVED5] =  */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_LLD] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LDC1] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LDC2] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_LD] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SC] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SWC1] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SWC2] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_RESERVED6] =  */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_OPCODE_SCD] =  */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SDC1] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SDC2] =  */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE),
    /* [FAULT_INST_OPCODE_SD] =  */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RT_OFFSET_BASE)
};

const char *sSpecialStrs[] = {
    /* [FAULT_INST_SPECIAL_SLL] = */        "SLL",
    /* [FAULT_INST_SPECIAL_RESERVED0] = */  "RESERVED0",
    /* [FAULT_INST_SPECIAL_SRL] = */        "SRL",
    /* [FAULT_INST_SPECIAL_SRA] = */        "SRA",
    /* [FAULT_INST_SPECIAL_SLLV] = */       "SLLV",
    /* [FAULT_INST_SPECIAL_RESERVED1] = */  "RESERVED1",
    /* [FAULT_INST_SPECIAL_SRLV] = */       "SRLV",
    /* [FAULT_INST_SPECIAL_SRAV] = */       "SRAV",
    /* [FAULT_INST_SPECIAL_JR] = */         "JR",
    /* [FAULT_INST_SPECIAL_JALR] = */       "JALR",
    /* [FAULT_INST_SPECIAL_RESERVED2] = */  "RESERVED2",
    /* [FAULT_INST_SPECIAL_RESERVED3] = */  "RESERVED3",
    /* [FAULT_INST_SPECIAL_SYSCALL] = */    "SYSCALL",
    /* [FAULT_INST_SPECIAL_BREAK] = */      "BREAK",
    /* [FAULT_INST_SPECIAL_RESERVED4] = */  "RESERVED4",
    /* [FAULT_INST_SPECIAL_SYNC] = */       "SYNC",
    /* [FAULT_INST_SPECIAL_MFHI] = */       "MFHI",
    /* [FAULT_INST_SPECIAL_MTHI] = */       "MTHI",
    /* [FAULT_INST_SPECIAL_MFLO] = */       "MFLO",
    /* [FAULT_INST_SPECIAL_MTLO] = */       "MTLO",
    /* [FAULT_INST_SPECIAL_DSLLV] = */      "DSLLV",
    /* [FAULT_INST_SPECIAL_RESERVED5] = */  "RESERVED5",
    /* [FAULT_INST_SPECIAL_DSRLV] = */      "DSRLV",
    /* [FAULT_INST_SPECIAL_DSRAV] = */      "DSRAV",
    /* [FAULT_INST_SPECIAL_MULT] = */       "MULT",
    /* [FAULT_INST_SPECIAL_MULTU] = */      "MULTU",
    /* [FAULT_INST_SPECIAL_DIV] = */        "DIV",
    /* [FAULT_INST_SPECIAL_DIVU] = */       "DIVU",
    /* [FAULT_INST_SPECIAL_DMULT] = */      "DMULT",
    /* [FAULT_INST_SPECIAL_DMULTU] = */     "DMULTU",
    /* [FAULT_INST_SPECIAL_DDIV] = */       "DDIV",
    /* [FAULT_INST_SPECIAL_DDIVU] = */      "DDIVU",
    /* [FAULT_INST_SPECIAL_ADD] = */        "ADD",
    /* [FAULT_INST_SPECIAL_ADDU] = */       "ADDU",
    /* [FAULT_INST_SPECIAL_SUB] = */        "SUB",
    /* [FAULT_INST_SPECIAL_SUBU] = */       "SUBU",
    /* [FAULT_INST_SPECIAL_AND] = */        "AND",
    /* [FAULT_INST_SPECIAL_OR] = */         "OR",
    /* [FAULT_INST_SPECIAL_XOR] = */        "XOR",
    /* [FAULT_INST_SPECIAL_NOR] = */        "NOR",
    /* [FAULT_INST_SPECIAL_RESERVED6] = */  "RESERVED6",
    /* [FAULT_INST_SPECIAL_RESERVED7] = */  "RESERVED7",
    /* [FAULT_INST_SPECIAL_SLT] = */        "SLT",
    /* [FAULT_INST_SPECIAL_SLTU] = */       "SLTU",
    /* [FAULT_INST_SPECIAL_DADD] = */       "DADD",
    /* [FAULT_INST_SPECIAL_DADDU] = */      "DADDU",
    /* [FAULT_INST_SPECIAL_DSUB] = */       "DSUB",
    /* [FAULT_INST_SPECIAL_DSUBU] = */      "DSUBU",
    /* [FAULT_INST_SPECIAL_TGE] = */        "TGE",
    /* [FAULT_INST_SPECIAL_TGEU] = */       "TGEU",
    /* [FAULT_INST_SPECIAL_TLT] = */        "TLT",
    /* [FAULT_INST_SPECIAL_TLTU] = */       "TLTU",
    /* [FAULT_INST_SPECIAL_TEQ] = */        "TEQ",
    /* [FAULT_INST_SPECIAL_RESERVED8] = */  "RESERVED8",
    /* [FAULT_INST_SPECIAL_TNE] = */        "TNE",
    /* [FAULT_INST_SPECIAL_RESERVED9] = */  "RESERVED9",
    /* [FAULT_INST_SPECIAL_DSLL] = */       "DSLL",
    /* [FAULT_INST_SPECIAL_RESERVED10] = */ "RESERVED10",
    /* [FAULT_INST_SPECIAL_DSRL] = */       "DSRL",
    /* [FAULT_INST_SPECIAL_DSRA] = */       "DSRA",
    /* [FAULT_INST_SPECIAL_DSLL32] = */     "DSLL32",
    /* [FAULT_INST_SPECIAL_RESERVED11] = */ "RESERVED11",
    /* [FAULT_INST_SPECIAL_DSRL32] = */     "DSRL32",
    /* [FAULT_INST_SPECIAL_DSRA32] = */     "DSRA32"
};

struct FaultInstInfo sSpecialInstrInfo[] = {
    /* [FAULT_INST_SPECIAL_SLL] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_SA),
    /* [FAULT_INST_SPECIAL_RESERVED0] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_SRL] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_SA),
    /* [FAULT_INST_SPECIAL_SRA] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_SA),
    /* [FAULT_INST_SPECIAL_SLLV] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_RS),
    /* [FAULT_INST_SPECIAL_RESERVED1] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_SRLV] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_RS),
    /* [FAULT_INST_SPECIAL_SRAV] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_RS),
    /* [FAULT_INST_SPECIAL_JR] = */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS),
    /* [FAULT_INST_SPECIAL_JALR] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RD),
    /* [FAULT_INST_SPECIAL_RESERVED2] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_RESERVED3] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_SYSCALL] = */    FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_BREAK] = */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_RESERVED4] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_SYNC] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_MFHI] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD),
    /* [FAULT_INST_SPECIAL_MTHI] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS),
    /* [FAULT_INST_SPECIAL_MFLO] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD),
    /* [FAULT_INST_SPECIAL_MTLO] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS),
    /* [FAULT_INST_SPECIAL_DSLLV] = */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_RS),
    /* [FAULT_INST_SPECIAL_RESERVED5] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_DSRLV] = */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_RS),
    /* [FAULT_INST_SPECIAL_DSRAV] = */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_RS),
    /* [FAULT_INST_SPECIAL_MULT] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_MULTU] = */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_DIV] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_DIVU] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_DMULT] = */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_DMULTU] = */     FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_DDIV] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_DDIVU] = */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_ADD] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_ADDU] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_SUB] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_SUBU] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_AND] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_OR] = */         FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_XOR] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_NOR] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_RESERVED6] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_RESERVED7] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_SLT] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_SLTU] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_DADD] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_DADDU] = */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_DSUB] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_DSUBU] = */      FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RS_RT),
    /* [FAULT_INST_SPECIAL_TGE] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_TGEU] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_TLT] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_TLTU] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_TEQ] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_RESERVED8] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_TNE] = */        FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RS_RT),
    /* [FAULT_INST_SPECIAL_RESERVED9] = */  FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_DSLL] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_SA),
    /* [FAULT_INST_SPECIAL_RESERVED10] = */ FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_DSRL] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_SA),
    /* [FAULT_INST_SPECIAL_DSRA] = */       FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_SA),
    /* [FAULT_INST_SPECIAL_DSLL32] = */     FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_SA),
    /* [FAULT_INST_SPECIAL_RESERVED11] = */ FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_NONE),
    /* [FAULT_INST_SPECIAL_DSRL32] = */     FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_SA),
    /* [FAULT_INST_SPECIAL_DSRA32] = */     FAULT_INST_INFO_DEF(FAULT_INST_FORMAT_STR_RD_RT_SA)
};

const char *sRegimmStrs[] = {
    /* [FAULT_INST_REGIMMS_BLTZ] = */       "BLTZ",
    /* [FAULT_INST_REGIMMS_BGEZ] = */       "BGEZ",
    /* [FAULT_INST_REGIMMS_BLTZL] = */      "BLTZL",
    /* [FAULT_INST_REGIMMS_BGEZL] = */      "BGEZL",
    /* [FAULT_INST_REGIMMS_RESERVED0] = */  "RESERVED0",
    /* [FAULT_INST_REGIMMS_RESERVED1] = */  "RESERVED1",
    /* [FAULT_INST_REGIMMS_RESERVED2] = */  "RESERVED2",
    /* [FAULT_INST_REGIMMS_RESERVED3] = */  "RESERVED3",
    /* [FAULT_INST_REGIMMS_TGEI] = */       "TGEI",
    /* [FAULT_INST_REGIMMS_TGEIU] = */      "TGEIU",
    /* [FAULT_INST_REGIMMS_TLTI] = */       "TLTI",
    /* [FAULT_INST_REGIMMS_TLTIU] = */      "TLTIU",
    /* [FAULT_INST_REGIMMS_TEQI] = */       "TEQI",
    /* [FAULT_INST_REGIMMS_RESERVED4] = */  "RESERVED4",
    /* [FAULT_INST_REGIMMS_TNEI] = */       "TNEI",
    /* [FAULT_INST_REGIMMS_RESERVED5] = */  "RESERVED5",
    /* [FAULT_INST_REGIMMS_BLTZAL] = */     "BLTZAL",
    /* [FAULT_INST_REGIMMS_BGEZAL] = */     "BGEZAL",
    /* [FAULT_INST_REGIMMS_BLTZALL] = */    "BLTZALL",
    /* [FAULT_INST_REGIMMS_BGEZALL] = */    "BGEZALL",
    /* [FAULT_INST_REGIMMS_RESERVED6] = */  "RESERVED6",
    /* [FAULT_INST_REGIMMS_RESERVED7] = */  "RESERVED7",
    /* [FAULT_INST_REGIMMS_RESERVED8] = */  "RESERVED8",
    /* [FAULT_INST_REGIMMS_RESERVED9] = */  "RESERVED9",
    /* [FAULT_INST_REGIMMS_RESERVED10] = */ "RESERVED10",
    /* [FAULT_INST_REGIMMS_RESERVED11] = */ "RESERVED11",
    /* [FAULT_INST_REGIMMS_RESERVED12] = */ "RESERVED12",
    /* [FAULT_INST_REGIMMS_RESERVED13] = */ "RESERVED13",
    /* [FAULT_INST_REGIMMS_RESERVED14] = */ "RESERVED14",
    /* [FAULT_INST_REGIMMS_RESERVED15] = */ "RESERVED15",
    /* [FAULT_INST_REGIMMS_RESERVED16] = */ "RESERVED16",
    /* [FAULT_INST_REGIMMS_RESERVED17] = */ "RESERVED17"
};

const char *sCopzrsStrs[] = {
    /* [FAULT_INST_COPZRS_MF] = */          "MF",
    /* [FAULT_INST_COPZRS_DMF] = */         "DMF",
    /* [FAULT_INST_COPZRS_CF] = */          "CF",
    /* [FAULT_INST_COPZRS_RESERVED0] = */   "RESERVED0",
    /* [FAULT_INST_COPZRS_MT] = */          "MT",
    /* [FAULT_INST_COPZRS_DMT] = */         "DMT",
    /* [FAULT_INST_COPZRS_CT] = */          "CT",
    /* [FAULT_INST_COPZRS_RESERVED1] = */   "RESERVED1",
    /* [FAULT_INST_COPZRS_BC] = */          "BC",
    /* [FAULT_INST_COPZRS_RESERVED2] = */   "RESERVED2",
    /* [FAULT_INST_COPZRS_RESERVED3] = */   "RESERVED3",
    /* [FAULT_INST_COPZRS_RESERVED4] = */   "RESERVED4",
    /* [FAULT_INST_COPZRS_RESERVED5] = */   "RESERVED5",
    /* [FAULT_INST_COPZRS_RESERVED6] = */   "RESERVED6",
    /* [FAULT_INST_COPZRS_RESERVED7] = */   "RESERVED7",
    /* [FAULT_INST_COPZRS_RESERVED8] = */   "RESERVED8"
};

const char *sCopzrtStrs[] = {
    /* [FAULT_INST_COPZRT_BCF] = */         "BCF",
    /* [FAULT_INST_COPZRT_BCT] = */         "BCT",
    /* [FAULT_INST_COPZRT_BCFL] = */        "BCFL",
    /* [FAULT_INST_COPZRT_BCTL] = */        "BCTL",
    /* [FAULT_INST_COPZRT_RESERVED0] = */   "RESERVED0",
    /* [FAULT_INST_COPZRT_RESERVED1] = */   "RESERVED1",
    /* [FAULT_INST_COPZRT_RESERVED2] = */   "RESERVED2",
    /* [FAULT_INST_COPZRT_RESERVED3] = */   "RESERVED3",
    /* [FAULT_INST_COPZRT_RESERVED4] = */   "RESERVED4",
    /* [FAULT_INST_COPZRT_RESERVED5] = */   "RESERVED5",
    /* [FAULT_INST_COPZRT_RESERVED6] = */   "RESERVED6",
    /* [FAULT_INST_COPZRT_RESERVED7] = */   "RESERVED7",
    /* [FAULT_INST_COPZRT_RESERVED8] = */   "RESERVED8",
    /* [FAULT_INST_COPZRT_RESERVED9] = */   "RESERVED9",
    /* [FAULT_INST_COPZRT_RESERVED10] = */  "RESERVED10",
    /* [FAULT_INST_COPZRT_RESERVED11] = */  "RESERVED11",
    /* [FAULT_INST_COPZRT_RESERVED12] = */  "RESERVED12",
    /* [FAULT_INST_COPZRT_RESERVED13] = */  "RESERVED13",    
    /* [FAULT_INST_COPZRT_RESERVED14] = */  "RESERVED14",    
    /* [FAULT_INST_COPZRT_RESERVED15] = */  "RESERVED15",    
    /* [FAULT_INST_COPZRT_RESERVED16] = */  "RESERVED16",    
    /* [FAULT_INST_COPZRT_RESERVED17] = */  "RESERVED17",    
    /* [FAULT_INST_COPZRT_RESERVED18] = */  "RESERVED18",    
    /* [FAULT_INST_COPZRT_RESERVED19] = */  "RESERVED19",    
    /* [FAULT_INST_COPZRT_RESERVED20] = */  "RESERVED20",    
    /* [FAULT_INST_COPZRT_RESERVED21] = */  "RESERVED21",    
    /* [FAULT_INST_COPZRT_RESERVED22] = */  "RESERVED22",    
    /* [FAULT_INST_COPZRT_RESERVED23] = */  "RESERVED23",    
    /* [FAULT_INST_COPZRT_RESERVED24] = */  "RESERVED24",    
    /* [FAULT_INST_COPZRT_RESERVED25] = */  "RESERVED25",    
    /* [FAULT_INST_COPZRT_RESERVED26] = */  "RESERVED26",    
    /* [FAULT_INST_COPZRT_RESERVED27] = */  "RESERVED27"
};

const char *sCp0Strs[] = {
    /* [FAULT_INST_CP0_RESERVED0] = */      "RESERVED0",
    /* [FAULT_INST_CP0_TLBR] = */           "TLBR",
    /* [FAULT_INST_CP0_TLBWI] = */          "TLBWI",
    /* [FAULT_INST_CP0_RESERVED1] = */      "RESERVED1",
    /* [FAULT_INST_CP0_RESERVED2] = */      "RESERVED2",
    /* [FAULT_INST_CP0_RESERVED3] = */      "RESERVED3",
    /* [FAULT_INST_CP0_TLBWR] = */          "TLBWR",
    /* [FAULT_INST_CP0_RESERVED4] = */      "RESERVED4",
    /* [FAULT_INST_CP0_TLBP] = */           "TLBP",
    /* [FAULT_INST_CP0_RESERVED5] = */      "RESERVED5",
    /* [FAULT_INST_CP0_RESERVED6] = */      "RESERVED6",
    /* [FAULT_INST_CP0_RESERVED7] = */      "RESERVED7",
    /* [FAULT_INST_CP0_RESERVED8] = */      "RESERVED8",
    /* [FAULT_INST_CP0_RESERVED9] = */      "RESERVED9",
    /* [FAULT_INST_CP0_RESERVED10] = */     "RESERVED10",
    /* [FAULT_INST_CP0_RESERVED11] = */     "RESERVED11",
    /* [FAULT_INST_CP0_RESERVED12] = */     "RESERVED12",
    /* [FAULT_INST_CP0_RESERVED13] = */     "RESERVED13",
    /* [FAULT_INST_CP0_RESERVED14] = */     "RESERVED14",
    /* [FAULT_INST_CP0_RESERVED15] = */     "RESERVED15",
    /* [FAULT_INST_CP0_RESERVED16] = */     "RESERVED16",
    /* [FAULT_INST_CP0_RESERVED17] = */     "RESERVED17",
    /* [FAULT_INST_CP0_RESERVED18] = */     "RESERVED18",
    /* [FAULT_INST_CP0_RESERVED19] = */     "RESERVED19",
    /* [FAULT_INST_CP0_ERET] = */           "ERET",
    /* [FAULT_INST_CP0_RESERVED20] = */     "RESERVED20",
    /* [FAULT_INST_CP0_RESERVED21] = */     "RESERVED21",
    /* [FAULT_INST_CP0_RESERVED22] = */     "RESERVED22",
    /* [FAULT_INST_CP0_RESERVED23] = */     "RESERVED23",
    /* [FAULT_INST_CP0_RESERVED24] = */     "RESERVED24",
    /* [FAULT_INST_CP0_RESERVED25] = */     "RESERVED25",
    /* [FAULT_INST_CP0_RESERVED26] = */     "RESERVED26",
    /* [FAULT_INST_CP0_RESERVED27] = */     "RESERVED27",
    /* [FAULT_INST_CP0_RESERVED28] = */     "RESERVED28",
    /* [FAULT_INST_CP0_RESERVED29] = */     "RESERVED29",
    /* [FAULT_INST_CP0_RESERVED30] = */     "RESERVED30",
    /* [FAULT_INST_CP0_RESERVED31] = */     "RESERVED31",
    /* [FAULT_INST_CP0_RESERVED32] = */     "RESERVED32",
    /* [FAULT_INST_CP0_RESERVED33] = */     "RESERVED33",
    /* [FAULT_INST_CP0_RESERVED34] = */     "RESERVED34",
    /* [FAULT_INST_CP0_RESERVED35] = */     "RESERVED35",
    /* [FAULT_INST_CP0_RESERVED36] = */     "RESERVED36",
    /* [FAULT_INST_CP0_RESERVED37] = */     "RESERVED37",
    /* [FAULT_INST_CP0_RESERVED38] = */     "RESERVED38",
    /* [FAULT_INST_CP0_RESERVED39] = */     "RESERVED39",
    /* [FAULT_INST_CP0_RESERVED40] = */     "RESERVED40",
    /* [FAULT_INST_CP0_RESERVED41] = */     "RESERVED41",
    /* [FAULT_INST_CP0_RESERVED42] = */     "RESERVED42",
    /* [FAULT_INST_CP0_RESERVED43] = */     "RESERVED43",
    /* [FAULT_INST_CP0_RESERVED44] = */     "RESERVED44",
    /* [FAULT_INST_CP0_RESERVED45] = */     "RESERVED45",
    /* [FAULT_INST_CP0_RESERVED46] = */     "RESERVED46",
    /* [FAULT_INST_CP0_RESERVED47] = */     "RESERVED47",
    /* [FAULT_INST_CP0_RESERVED48] = */     "RESERVED48",
    /* [FAULT_INST_CP0_RESERVED49] = */     "RESERVED49",
    /* [FAULT_INST_CP0_RESERVED50] = */     "RESERVED50",
    /* [FAULT_INST_CP0_RESERVED51] = */     "RESERVED51",
    /* [FAULT_INST_CP0_RESERVED52] = */     "RESERVED52",
    /* [FAULT_INST_CP0_RESERVED53] = */     "RESERVED53",
    /* [FAULT_INST_CP0_RESERVED54] = */     "RESERVED54",
    /* [FAULT_INST_CP0_RESERVED55] = */     "RESERVED55",
    /* [FAULT_INST_CP0_RESERVED56] = */     "RESERVED56",
    /* [FAULT_INST_CP0_RESERVED57] = */     "RESERVED57"
};

void Fault_SleepImpl(u32 msec) {
    OSTime value = (msec * OS_CPU_COUNTER) / 1000ULL;

    csleep(value);
}

/**
 * Registers a fault client.
 *
 * Clients contribute at least one page to the crash screen, drawn by `callback`.
 * Arguments are passed on to the callback through `arg0` and `arg1`.
 */
void Fault_AddClient(FaultClient* client, FaultClientCallback callback, void* arg0, void* arg1) {
    OSIntMask mask;
    u32 alreadyExists = false;

    mask = osSetIntMask(OS_IM_NONE);

    // Ensure the client is not already registered
    {
        FaultClient* iter = sFaultInstance->first_client;

        while (iter != NULL) {
            if (iter == client) {
                alreadyExists = true;
                goto end;
            }
            iter = iter->next;
        }
    }

    client->next = NULL;
    client->prev = NULL;

    client->callback = callback;
    client->arg0 = arg0;
    client->arg1 = arg1;

    if(sFaultInstance->first_client == NULL)
    {
        sFaultInstance->first_client = client;
    }
    else
    {
        sFaultInstance->last_client->next = client;
        client->prev = sFaultInstance->last_client;
    }

    sFaultInstance->last_client = client;
    
    // client->next = sFaultInstance->clients;
    // sFaultInstance->clients = client;

end:

    osSetIntMask(mask);

    if (alreadyExists) {
        osSyncPrintf(VT_COL(RED, WHITE) "fault_AddClient: %08x は既にリスト中にある\n" VT_RST, client);
    }
}

/**
 * Removes a fault client so that the page is no longer displayed if a crash occurs.
 */
void Fault_RemoveClient(FaultClient* client) {
    FaultClient* iter = sFaultInstance->first_client;
    // FaultClient* lastIter = NULL;
    OSIntMask mask;
    u32 listIsEmpty = false;

    mask = osSetIntMask(OS_IM_NONE);

    while (iter) 
    {
        if (iter == client) {
            if(client->prev != NULL)
            {
                client->prev->next = client->next;
            }
            else
            {
                sFaultInstance->first_client = client->next;

                if(sFaultInstance->first_client != NULL)
                {
                    sFaultInstance->first_client->prev = NULL;
                }
            }

            if(client->next != NULL)
            {
                client->next->prev = client->prev;
            }
            else
            {
                sFaultInstance->last_client = client->prev;

                if(sFaultInstance->last_client != NULL)
                {
                    sFaultInstance->last_client->next = NULL;
                }
            }

            break;
            // if (lastIter != NULL) {
            //     lastIter->next = client->next;
            // } else {
            //     sFaultInstance->clients = client;
            //     if (sFaultInstance->clients) {
            //         sFaultInstance->clients = client->next;
            //     } else {
            //         listIsEmpty = 1;
            //     }
            // }
            // break;
        }

        // lastIter = iter;
        iter = iter->next;
    }

    osSetIntMask(mask);

    if (listIsEmpty) {
        osSyncPrintf(VT_COL(RED, WHITE) "fault_RemoveClient: %08x リスト不整合です\n" VT_RST, client);
    }
}

/**
 * Registers an address converter client. This enables the crash screen to look up virtual
 * addresses of overlays relocated during runtime. Address conversion is carried out by
 * `callback`, which either returns a virtual address or NULL if the address could not
 * be converted.
 *
 * The callback is intended to be
 * `uintptr_t (*callback)(uintptr_t addr, void* arg)`
 * The callback may return 0 if it could not convert the address
 */
void Fault_AddAddrConvClient(FaultAddrConvClient* client, FaultAddrConvClientCallback callback, void* arg) {
    OSIntMask mask;
    s32 alreadyExists = false;

    mask = osSetIntMask(OS_IM_NONE);

    {
        FaultAddrConvClient* iter = sFaultInstance->first_addr_conv_client;

        while (iter != NULL) {
            if (iter == client) {
                alreadyExists = true;
                goto end;
            }
            iter = iter->next;
        }
    }

    client->callback = callback;
    client->arg = arg;

    if(sFaultInstance->first_addr_conv_client == NULL)
    {
        sFaultInstance->first_addr_conv_client = client;
    }
    else
    {
        sFaultInstance->last_addr_conv_client->next = client;
        client->prev = sFaultInstance->last_addr_conv_client;
    }

    sFaultInstance->last_addr_conv_client = client;

    // client->next = sFaultInstance->addrConvClients;
    // sFaultInstance->addrConvClients = client;

end:
    osSetIntMask(mask);

    if (alreadyExists) {
        osSyncPrintf(VT_COL(RED, WHITE) "fault_AddressConverterAddClient: %08x は既にリスト中にある\n" VT_RST, client);
    }
}

void Fault_RemoveAddrConvClient(FaultAddrConvClient* client) {
    FaultAddrConvClient* iter = sFaultInstance->first_addr_conv_client;
    // FaultAddrConvClient* lastIter = NULL;
    OSIntMask mask;
    bool listIsEmpty = false;

    mask = osSetIntMask(OS_IM_NONE);

    while (iter) {
        if (iter == client) {
            if(client->prev != NULL)
            {
                client->prev->next = client->next;
            }
            else
            {
                sFaultInstance->first_addr_conv_client = client->next;
            }

            if(client->next != NULL)
            {
                client->next->prev = client->prev;
            }
            else
            {
                sFaultInstance->last_addr_conv_client = client->prev;
            }
        }

        // lastIter = iter;
        iter = iter->next;
    }

    osSetIntMask(mask);

    if (listIsEmpty) {
        osSyncPrintf(VT_COL(RED, WHITE) "fault_AddressConverterRemoveClient: %08x は既にリスト中にある\n" VT_RST,
                     client);
    }
}

/**
 * Converts `addr` to a virtual address via the registered
 * address converter clients
 */
uintptr_t Fault_ConvertAddress(uintptr_t addr) {
    uintptr_t ret;
    FaultAddrConvClient* iter = sFaultInstance->first_addr_conv_client;

    while (iter != NULL) {
        if (iter->callback != NULL) {
            ret = iter->callback(addr, iter->arg);
            if (ret != 0) {
                return ret;
            }
        }
        iter = iter->next;
    }

    return 0;
}

void Fault_Sleep(u32 msec) {
    Fault_SleepImpl(msec);
}

void Fault_PadCallback(Input* input) {
    PadMgr_GetInput2(input, false);
}

void Fault_UpdatePadImpl(void) {
    sFaultInstance->padCallback(sFaultInstance->inputs);
}

/**
 * Awaits user input
 *
 * L toggles auto-scroll
 * DPad-Up enables osSyncPrintf output
 * DPad-Down disables osSyncPrintf output
 * A and DPad-Right continues and returns true
 * DPad-Left continues and returns false
 */
u32 Fault_WaitForInputImpl(void) {
    Input* input = &sFaultInstance->inputs[0];
    s32 count = 600;
    u32 pressedBtn;

    while (true) {
        Fault_Sleep(1000 / 60);
        Fault_UpdatePadImpl();

        pressedBtn = input->press.button;

        if (pressedBtn == BTN_L) {
            sFaultInstance->autoScroll = !sFaultInstance->autoScroll;
        }

        if (sFaultInstance->autoScroll) {
            if (count-- < 1) {
                return false;
            }
        } else {
            if ((pressedBtn == BTN_A) || (pressedBtn == BTN_DRIGHT)) {
                return false;
            }

            if (pressedBtn == BTN_DLEFT) {
                return true;
            }

            if (pressedBtn == BTN_DUP) {
                FaultDrawer_SetOsSyncPrintfEnabled(true);
            }

            if (pressedBtn == BTN_DDOWN) {
                FaultDrawer_SetOsSyncPrintfEnabled(false);
            }
        }
    }
}

u32 Fault_WaitForInput2(void)
{
    Input* input = &sFaultInstance->inputs[0];

    do 
    {
        Fault_Sleep(1000 / 60);
        Fault_UpdatePadImpl();
    }
    while(input->press.button == 0);

    return input->cur.button;
}

void Fault_WaitForInput(void) {
    Fault_WaitForInput2();
    // Fault_WaitForInputImpl();
}

void Fault_DrawRec(s32 x, s32 y, s32 w, s32 h, u16 color) {
    FaultDrawer_DrawRecImpl(x, y, x + w - 1, y + h - 1, color);
}

void Fault_FillScreenBlack(void) {
    FaultDrawer_SetForeColor(GPACK_RGBA5551(255, 255, 255, 1));
    FaultDrawer_SetBackColor(GPACK_RGBA5551(0, 0, 0, 1));
    FaultDrawer_FillScreen();
    FaultDrawer_SetBackColor(GPACK_RGBA5551(0, 0, 0, 0));
}

void Fault_FillScreenRed(void) {
    FaultDrawer_SetForeColor(GPACK_RGBA5551(255, 255, 255, 1));
    FaultDrawer_SetBackColor(GPACK_RGBA5551(240, 0, 0, 1));
    FaultDrawer_FillScreen();
    FaultDrawer_SetBackColor(GPACK_RGBA5551(0, 0, 0, 0));
}

void Fault_DrawCornerRec(u16 color) {
    Fault_DrawRec(22, 16, 8, 1, color);
}

void Fault_PrintFReg(s32 index, f32* value) {
    u32 raw = *(u32*)value;
    s32 exp = ((raw & 0x7F800000) >> 23) - 127;

    if (((exp > -127) && (exp <= 127)) || (raw == 0)) {
        FaultDrawer_Printf("F%02d:%14.7e ", index, *value);
    } else {
        // Print subnormal floats as their IEEE-754 hex representation
        FaultDrawer_Printf("F%02d:  %08x(16) ", index, raw);
    }
}

void Fault_LogFReg(s32 index, f32* value) {
    u32 raw = *(u32*)value;
    s32 exp = ((raw & 0x7F800000) >> 23) - 127;

    if (((exp > -127) && (exp <= 127)) || (raw == 0)) {
        osSyncPrintf("F%02d:%14.7e ", index, *value);
    } else {
        osSyncPrintf("F%02d:  %08x(16) ", index, *(u32*)value);
    }
}

void Fault_PrintFPCSR(u32 value) {
    s32 i;
    u32 flag = FPCSR_CE;

    FaultDrawer_Printf("FPCSR:%08xH ", value);

    // Go through each of the six causes and print the name of
    // the first cause that is set
    for (i = 0; i < ARRAY_COUNT(sFpuExceptions); i++) {
        if (value & flag) {
            FaultDrawer_Printf("(%s)", sFpuExceptions[i]);
            break;
        }
        flag >>= 1;
    }
    FaultDrawer_Printf("\n");
}

void Fault_LogFPCSR(u32 value) {
    s32 i;
    u32 flag = FPCSR_CE;

    osSyncPrintf("FPCSR:%08xH  ", value);
    for (i = 0; i < ARRAY_COUNT(sFpuExceptions); i++) {
        if (value & flag) {
            osSyncPrintf("(%s)\n", sFpuExceptions[i]);
            break;
        }
        flag >>= 1;
    }
}

#define EXC(code) (EXC_##code >> CAUSE_EXCSHIFT)

void Fault_PrintThreadContextInfo(OSThread* thread)
{
    __OSThreadContext* threadCtx;
    s16 causeStrIndex = _SHIFTR((u32)thread->context.cause, 2, 5);
    u32 index;

    if (causeStrIndex == EXC(WATCH)) { // Watchpoint
        causeStrIndex = 16;
    }
    if (causeStrIndex == EXC(VCED)) { // Virtual coherency on data
        causeStrIndex = 17;
    }

    FaultDrawer_FillScreen();
    FaultDrawer_SetForeColor(GPACK_RGBA5551(255, 255, 255, 1));
    FaultDrawer_SetBackColor(GPACK_RGBA5551(0, 0, 0, 0));
    FaultDrawer_SetCharPad(-2, 4);
    FaultDrawer_SetCursor(22, 20);

    threadCtx = &thread->context;
    FaultDrawer_Printf("Well, this shit crashed...\n");
    FaultDrawer_Printf("THREAD:%d (%d:%s)\n", thread->id, causeStrIndex, sCpuExceptions[causeStrIndex]);
    FaultDrawer_SetCharPad(-1, 0);

    FaultDrawer_Printf("PC:%08xH SR:%08xH VA:%08xH\n", (u32)threadCtx->pc, (u32)threadCtx->sr,
                       (u32)threadCtx->badvaddr);
    FaultDrawer_Printf("AT:%08xH V0:%08xH V1:%08xH\n", (u32)threadCtx->at, (u32)threadCtx->v0, (u32)threadCtx->v1);
    FaultDrawer_Printf("A0:%08xH A1:%08xH A2:%08xH\n", (u32)threadCtx->a0, (u32)threadCtx->a1, (u32)threadCtx->a2);
    FaultDrawer_Printf("A3:%08xH T0:%08xH T1:%08xH\n", (u32)threadCtx->a3, (u32)threadCtx->t0, (u32)threadCtx->t1);
    FaultDrawer_Printf("T2:%08xH T3:%08xH T4:%08xH\n", (u32)threadCtx->t2, (u32)threadCtx->t3, (u32)threadCtx->t4);
    FaultDrawer_Printf("T5:%08xH T6:%08xH T7:%08xH\n", (u32)threadCtx->t5, (u32)threadCtx->t6, (u32)threadCtx->t7);
    FaultDrawer_Printf("S0:%08xH S1:%08xH S2:%08xH\n", (u32)threadCtx->s0, (u32)threadCtx->s1, (u32)threadCtx->s2);
    FaultDrawer_Printf("S3:%08xH S4:%08xH S5:%08xH\n", (u32)threadCtx->s3, (u32)threadCtx->s4, (u32)threadCtx->s5);
    FaultDrawer_Printf("S6:%08xH S7:%08xH T8:%08xH\n", (u32)threadCtx->s6, (u32)threadCtx->s7, (u32)threadCtx->t8);
    FaultDrawer_Printf("T9:%08xH GP:%08xH SP:%08xH\n", (u32)threadCtx->t9, (u32)threadCtx->gp, (u32)threadCtx->sp);
    FaultDrawer_Printf("S8:%08xH RA:%08xH LO:%08xH\n\n", (u32)threadCtx->s8, (u32)threadCtx->ra, (u32)threadCtx->lo);

    FaultDrawer_Printf("\nActive codes:");
    for(index = 0; index < gChaosContext.active_code_count; index++)
    {
        struct ChaosCode *code = gChaosContext.active_codes + index;
        if(code->code != CHAOS_CODE_FAKE_CRASH)
        {
            FaultDrawer_Printf("\n%s", gChaosCodeNames[code->code]);
        }
    }

    if(Chaos_GetConfigFlag(CHAOS_CONFIG_SAVE_AT_GAME_CRASH))
    {
        FaultDrawer_SetForeColor(GPACK_RGBA5551(255, 255, 0, 1));
        FaultDrawer_SetCursor(22, 200);
        FaultDrawer_Printf("Your progress has been saved.\nYou can safely reset the console.");
    }
}

u32 Fault_PrintThreadContext(OSThread* thread) 
{
    Fault_PrintThreadContextInfo(thread);
    return Fault_WaitForInput2();
}

// void osSyncPrintfThreadContext(OSThread* thread) {
//     __OSThreadContext* threadCtx;
//     s16 causeStrIndex = _SHIFTR((u32)thread->context.cause, 2, 5);
//     u32 index;

//     if (causeStrIndex == EXC(WATCH)) { // Watchpoint
//         causeStrIndex = 16;
//     }
//     if (causeStrIndex == EXC(VCED)) { // Virtual coherency on data
//         causeStrIndex = 17;
//     }

//     threadCtx = &thread->context;
//     osSyncPrintf("Well, this shit crashed...\n");
//     osSyncPrintf("\n");
//     osSyncPrintf("THREAD ID:%d (%d:%s)\n", thread->id, causeStrIndex, sCpuExceptions[causeStrIndex]);

//     osSyncPrintf("PC:%08xH   SR:%08xH   VA:%08xH\n", (u32)threadCtx->pc, (u32)threadCtx->sr, (u32)threadCtx->badvaddr);
//     osSyncPrintf("AT:%08xH   V0:%08xH   V1:%08xH\n", (u32)threadCtx->at, (u32)threadCtx->v0, (u32)threadCtx->v1);
//     osSyncPrintf("A0:%08xH   A1:%08xH   A2:%08xH\n", (u32)threadCtx->a0, (u32)threadCtx->a1, (u32)threadCtx->a2);
//     osSyncPrintf("A3:%08xH   T0:%08xH   T1:%08xH\n", (u32)threadCtx->a3, (u32)threadCtx->t0, (u32)threadCtx->t1);
//     osSyncPrintf("T2:%08xH   T3:%08xH   T4:%08xH\n", (u32)threadCtx->t2, (u32)threadCtx->t3, (u32)threadCtx->t4);
//     osSyncPrintf("T5:%08xH   T6:%08xH   T7:%08xH\n", (u32)threadCtx->t5, (u32)threadCtx->t6, (u32)threadCtx->t7);
//     osSyncPrintf("S0:%08xH   S1:%08xH   S2:%08xH\n", (u32)threadCtx->s0, (u32)threadCtx->s1, (u32)threadCtx->s2);
//     osSyncPrintf("S3:%08xH   S4:%08xH   S5:%08xH\n", (u32)threadCtx->s3, (u32)threadCtx->s4, (u32)threadCtx->s5);
//     osSyncPrintf("S6:%08xH   S7:%08xH   T8:%08xH\n", (u32)threadCtx->s6, (u32)threadCtx->s7, (u32)threadCtx->t8);
//     osSyncPrintf("T9:%08xH   GP:%08xH   SP:%08xH\n", (u32)threadCtx->t9, (u32)threadCtx->gp, (u32)threadCtx->sp);
//     osSyncPrintf("S8:%08xH   RA:%08xH   LO:%08xH\n", (u32)threadCtx->s8, (u32)threadCtx->ra, (u32)threadCtx->lo);

//     osSyncPrintf("\nActive codes:");
//     for(index = 0; index < gChaosContext.active_code_count; index++)
//     {
//         struct ChaosCode *code = gChaosContext.active_codes + index;
//         osSyncPrintf("\n%s", gChaosCodeNames[code->code]);
//     }
//     // osSyncPrintf("\n");
//     // Fault_LogFPCSR(threadCtx->fpcsr);
//     // osSyncPrintf("\n");
//     // Fault_LogFReg(0, &threadCtx->fp0.f.f_even);
//     // Fault_LogFReg(2, &threadCtx->fp2.f.f_even);
//     // osSyncPrintf("\n");
//     // Fault_LogFReg(4, &threadCtx->fp4.f.f_even);
//     // Fault_LogFReg(6, &threadCtx->fp6.f.f_even);
//     // osSyncPrintf("\n");
//     // Fault_LogFReg(8, &threadCtx->fp8.f.f_even);
//     // Fault_LogFReg(10, &threadCtx->fp10.f.f_even);
//     // osSyncPrintf("\n");
//     // Fault_LogFReg(12, &threadCtx->fp12.f.f_even);
//     // Fault_LogFReg(14, &threadCtx->fp14.f.f_even);
//     // osSyncPrintf("\n");
//     // Fault_LogFReg(16, &threadCtx->fp16.f.f_even);
//     // Fault_LogFReg(18, &threadCtx->fp18.f.f_even);
//     // osSyncPrintf("\n");
//     // Fault_LogFReg(20, &threadCtx->fp20.f.f_even);
//     // Fault_LogFReg(22, &threadCtx->fp22.f.f_even);
//     // osSyncPrintf("\n");
//     // Fault_LogFReg(24, &threadCtx->fp24.f.f_even);
//     // Fault_LogFReg(26, &threadCtx->fp26.f.f_even);
//     // osSyncPrintf("\n");
//     // Fault_LogFReg(28, &threadCtx->fp28.f.f_even);
//     // Fault_LogFReg(30, &threadCtx->fp30.f.f_even);
//     // osSyncPrintf("\n");
// }

/**
 * Iterates through the active thread queue for a user thread with either
 * the CPU break or Fault flag set.
 */
OSThread* Fault_FindFaultedThread(void) {
    OSThread* iter = __osGetActiveQueue();

    // -1 indicates the end of the thread queue
    while (iter->priority != -1) {
        if ((iter->priority > OS_PRIORITY_IDLE) && (iter->priority < OS_PRIORITY_APPMAX) &&
            (iter->flags & (OS_FLAG_CPU_BREAK | OS_FLAG_FAULT))) {
            return iter;
        }
        iter = iter->tlnext;
    }

    return NULL;
}
void Fault_Wait5Seconds(void) {
    s32 pad;
    OSTime start = osGetTime();

    do {
        Fault_Sleep(1000 / 60);
    } while ((osGetTime() - start) <= OS_USEC_TO_CYCLES(5 * 1000 * 1000));

    sFaultInstance->autoScroll = true;
}

/**
 * Waits for the following button combination to be entered before returning:
 *
 * (DPad-Left & L & R & C-Right) & Start
 */
void Fault_WaitForButtonCombo(void) {
    Input* input = &sFaultInstance->inputs[0];

    FaultDrawer_SetForeColor(GPACK_RGBA5551(255, 255, 255, 1));
    FaultDrawer_SetBackColor(GPACK_RGBA5551(0, 0, 0, 1));

    do {
        do {
            Fault_Sleep(1000 / 60);
            Fault_UpdatePadImpl();
        } while (!CHECK_BTN_ALL(input->press.button, BTN_RESET));
    } while (!CHECK_BTN_ALL(input->cur.button, BTN_DLEFT | BTN_L | BTN_R | BTN_CRIGHT));
}

void Fault_DrawMemDumpContents(const char* title, uintptr_t addr, u32 param_3) {
    uintptr_t alignedAddr = addr;
    u32* writeAddr;
    s32 y;
    s32 x;

    // Ensure address is within the bounds of RDRAM (Fault_DrawMemDump has already done this)
    if (alignedAddr < K0BASE) {
        alignedAddr = K0BASE;
    }
    // 8MB RAM, leave room to display 0x100 bytes on the final page
    //! @bug The loop below draws 22 * 4 * 4 = 0x160 bytes per page. Due to this, by scrolling further than
    //! 0x807FFEA0 some invalid bytes are read from outside of 8MB RDRAM space. This does not cause a crash,
    //! however the values it displays are meaningless. On N64 hardware these invalid addresses are read as 0.
    if (alignedAddr > (K0BASE + 0x800000 - 0x100)) {
        alignedAddr = K0BASE + 0x800000 - 0x100;
    }

    // Ensure address is word-aligned
    alignedAddr &= ~3;
    writeAddr = (u32*)alignedAddr;

    Fault_FillScreenBlack();
    FaultDrawer_SetCharPad(-2, 0);

    FaultDrawer_DrawText(36, 18, "%s %08x", title ? title : "PrintDump", alignedAddr);

    if (alignedAddr >= K0BASE && alignedAddr < K2BASE) {
        for (y = 0; y < 22; y++) {
            FaultDrawer_DrawText(24, 28 + y * 9, "%06x", writeAddr);
            for (x = 0; x < 4; x++) {
                FaultDrawer_DrawText(82 + x * 52, 28 + y * 9, "%08x", *writeAddr++);
            }
        }
    }

    FaultDrawer_SetCharPad(0, 0);
}

/**
 * Draws the memory dump page.
 *
 * DPad-Up scrolls up.
 * DPad-Down scrolls down.
 * Holding A while scrolling speeds up scrolling by a factor of 0x10.
 * Holding B while scrolling speeds up scrolling by a factor of 0x100.
 *
 * L toggles auto-scrolling pages.
 * START and A move on to the next page.
 *
 * @param pc Program counter, pressing C-Up jumps to this address
 * @param sp Stack pointer, pressing C-Down jumps to this address
 * @param cLeftJump Unused parameter, pressing C-Left jumps to this address
 * @param cRightJump Unused parameter, pressing C-Right jumps to this address
 */
bool Fault_DrawMemDump(OSThread *thread, u32 button) {
    // s32 scrollCountdown;
    s32 off;
    Input* input = &sFaultInstance->inputs[0];
    uintptr_t addr = thread->context.pc;

    // if(just_changed)
    // {
    //     sFaultInstance->current_memdump_pc = thread->context.pc;
    // }

    // addr = sFaultInstance->current_memdump_pc;

    do 
    {
        // scrollCountdown = 0;
        // Ensure address is within the bounds of RDRAM
        if (addr < K0BASE) {
            addr = K0BASE;
        }
        // 8MB RAM, leave room to display 0x100 bytes on the final page
        if (addr > (K0BASE + 0x800000 - 0x100)) {
            addr = K0BASE + 0x800000 - 0x100;
        }

            // Align down the address to 0x10 bytes and draw the page contents
        addr &= ~0xF;
        Fault_DrawMemDumpContents("Dump", addr, 0);

            // scrollCountdown = 600;
            // while (sFaultInstance->autoScroll) {
            //     // Count down until it's time to move on to the next page
            //     if (scrollCountdown == 0) {
            //         return;
            //     }

            //     scrollCountdown--;

            //     Fault_Sleep(1000 / 60);
            //     Fault_UpdatePadImpl();

            //     if (CHECK_BTN_ALL(input->press.button, BTN_L)) {
            //         sFaultInstance->autoScroll = false;
            //     }
            // }

            // Wait for input
            // do {
            //     Fault_Sleep(1000 / 60);
            //     Fault_UpdatePadImpl();
            // } while (input->press.button == 0);

        button = Fault_WaitForInput2();

        // Move to next page
        if (CHECK_BTN_ANY(button, BTN_L | BTN_R)) 
        {
            return button;
        }

        // Memory dump controls

        off = 0x10;
        if (CHECK_BTN_ALL(button, BTN_A)) {
            off *= 0x10;
        }
        if (CHECK_BTN_ALL(button, BTN_B)) {
            off *= 0x100;
        }
        if (CHECK_BTN_ALL(button, BTN_DUP)) {
            addr -= off;
        }
        if (CHECK_BTN_ALL(button, BTN_DDOWN)) {
            addr += off;
        }
        if (CHECK_BTN_ALL(button, BTN_CUP)) {
            addr = thread->context.pc;
        }
        if (CHECK_BTN_ALL(button, BTN_CDOWN)) {
            addr = thread->context.sp;
        }

    }
    while(true);
    // } while (!CHECK_BTN_ALL(input->press.button, BTN_L));

    // Resume auto-scroll and move to next page
    // sFaultInstance->autoScroll = true;
}

void Fault_InstructionStr(char *instruction_str, const char *opcode_str, u32 instruction, u32 str_index, u32 pc)
{
    switch(str_index)
    {
        case FAULT_INST_FORMAT_STR_RT_OFFSET_BASE:
        {
            u8 base = (instruction >> 21) & 0x1f;
            u8 rt = (instruction >> 16) & 0x1f;
            u16 offset = instruction & 0xffff;
            u16 sign = offset & 0x8000;

            if(sign)
            {
                offset = (~offset) + 1;
            }

            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RT_OFFSET_BASE], 
                        opcode_str, sRegNames[rt], (sign ? "-" : " "), offset, sRegNames[base]);   
        }
        break;

        case FAULT_INST_FORMAT_STR_RT_RS_IMMEDIATE:
        {
            u8 rs = (instruction >> 21) & 0x1f;
            u8 rt = (instruction >> 16) & 0x1f;
            u16 offset = instruction & 0xffff;

            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RT_RS_IMMEDIATE], 
                opcode_str, sRegNames[rt], sRegNames[rs], offset);   
        }
        break;

        case FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE:
        {
            u8 rs = (instruction >> 21) & 0x1f;
            u8 rt = (instruction >> 16) & 0x1f;
            u16 offset = instruction & 0xffff;
            u16 sign = offset & 0x8000;

            if(sign)
            {
                offset = (~offset) + 1;
            }

            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RT_RS_SIMMEDIATE], 
                opcode_str, sRegNames[rt], sRegNames[rs], (sign ? "-" : " "), offset);   
        }
        break;

        case FAULT_INST_FORMAT_STR_RD_RS_RT:
        {
            u8 rs = (instruction >> 21) & 0x1f;
            u8 rt = (instruction >> 16) & 0x1f;
            u8 rd = (instruction >> 11) & 0x1f;
            
            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RD_RS_RT], 
                opcode_str, sRegNames[rd], sRegNames[rs], sRegNames[rt]);   
        }
        break;

        case FAULT_INST_FORMAT_STR_OP_OFFSET_BASE:
        {
            u8 base = (instruction >> 21) & 0x1f;
            u8 op = (instruction >> 16) & 0x1f;
            u16 offset = instruction & 0xffff;
            u16 sign = offset & 0x8000;

            if(sign)
            {
                offset = (~offset) + 1;
            }

            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_OP_OFFSET_BASE], 
                        opcode_str, op, (sign ? "-" : " "), offset, sRegNames[base]);   
        }
        break;

        case FAULT_INST_FORMAT_STR_TARGET:
        {
            u32 delay_slot_pc = pc + 4;
            u32 target = (delay_slot_pc & 0xc0000000) | ((instruction & 0x3ffffff) << 2);
            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_TARGET], 
                        opcode_str, target);   
        }
        break;

        case FAULT_INST_FORMAT_STR_RS_RT_OFFSET:
        {
            u8 rs = (instruction >> 21) & 0x1f;
            u8 rt = (instruction >> 16) & 0x1f;
            u16 offset = instruction & 0xffff;
            u16 sign = offset & 0x8000;

            if(sign)
            {
                offset = (~offset) + 1;
            }

            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RS_RT_OFFSET], 
                opcode_str, sRegNames[rs], sRegNames[rt], (sign ? "-" : " "), offset);   
        }
        break;

        case FAULT_INST_FORMAT_STR_RS_OFFSET:
        {
            u8 rs = (instruction >> 21) & 0x1f;
            u16 offset = instruction & 0xffff;
            u16 sign = offset & 0x8000;

            if(sign)
            {
                offset = (~offset) + 1;
            }

            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RS_OFFSET], 
                opcode_str, sRegNames[rs], (sign ? "-" : " "), offset);   
        }
        break;

        case FAULT_INST_FORMAT_STR_RS_RT:
        {
            u8 rs = (instruction >> 21) & 0x1f;
            u8 rt = (instruction >> 16) & 0x1f;

            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RS_RT], 
                opcode_str, sRegNames[rs], sRegNames[rt]);      
        }
        break;

        case FAULT_INST_FORMAT_STR_RD_RT_SA:
        {
            u8 rt = (instruction >> 16) & 0x1f;
            u8 rd = (instruction >> 11) & 0x1f;
            u8 sa = (instruction >> 6) & 0x1f;
            
            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RD_RT_SA], 
                opcode_str, sRegNames[rd], sRegNames[rt], sRegNames[sa]);   
        }
        break;

        case FAULT_INST_FORMAT_STR_RT_IMMEDIATE:
        {
            u8 rt = (instruction >> 16) & 0x1f;
            u16 immediate = instruction & 0xffff;
            
            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RT_IMMEDIATE], 
                opcode_str, sRegNames[rt], immediate);   
        }
        break;

        case FAULT_INST_FORMAT_STR_RS_IMMEDIATE:
        {
            u8 rs = (instruction >> 21) & 0x1f;
            u16 immediate = instruction & 0xffff;
            
            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RS_IMMEDIATE], 
                opcode_str, sRegNames[rs], immediate);   
        }
        break;

        default:
            strcpy(instruction_str, opcode_str);
        break;


        // case FAULT_INST_FORMAT_STR_RD_RT_RS:

        // break;

        

        // case FAULT_INST_FORMAT_STR_RD:

        // break;

        case FAULT_INST_FORMAT_STR_RS:
        {
            u8 rs = (instruction >> 21) & 0x1f;
            
            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RS], 
                opcode_str, sRegNames[rs]);   
        }
        break;

        case FAULT_INST_FORMAT_STR_RS_RD:
        {
            u8 rs = (instruction >> 21) & 0x1f;
            u8 rd = (instruction >> 11) & 0x1f;
            
            sprintf(instruction_str, sInstructionFormatStrs[FAULT_INST_FORMAT_STR_RS_RD], 
                opcode_str, sRegNames[rs], sRegNames[rd]);   
        }
        break;


    }
}

u32 Fault_Disasm(OSThread *thread, u32 button)
{
    u32 *pc = (u32 *)thread->context.pc;
    // u32 *pc = NULL;
    // u32 button;
    u32 multiplier = 1;
    char instruction_str[32];
    u32 line = 0;

    // if(just_changed)
    // {
    //     sFaultInstance->current_disasm_pc = thread->context.pc;
    // }

    // pc = (u32 *)sFaultInstance->current_disasm_pc;
    do
    {    

        Fault_FillScreenBlack();
        FaultDrawer_SetCharPad(-1, 0);
        FaultDrawer_Printf("Disasm: %08x\n", pc);

        FaultDrawer_SetCharPad(-2, 0);
        FaultDrawer_Printf("   PC   ");

        FaultDrawer_SetCharPad(-1, 0);
        FaultDrawer_Printf("|");

        FaultDrawer_SetCharPad(-2, 0);
        FaultDrawer_Printf("  INST  ");

        FaultDrawer_SetCharPad(-1, 0);
        FaultDrawer_Printf("|");

        FaultDrawer_SetCharPad(-2, 0);
        FaultDrawer_Printf("        ASM \n\n");
        // FaultDrawer_Printf("    PC   |   INST   |    ASM \n\n");

        // button = Fault_WaitForInput2();
        // button = sFaultInstance->inputs[0].press.button;

        for(line = 0; line < 20; line++)
        {
            u32 instruction = pc[line]; 
            u32 opcode = (instruction >> FAULT_INST_OPCODE_SHIFT) & FAULT_INST_OPCODE_MASK;

            switch(opcode)
            {
                case FAULT_INST_OPCODE_SPECIAL:
                {
                    u32 special_instruction = instruction & FAULT_INST_SPECIAL_MASK;
                    Fault_InstructionStr(instruction_str, sSpecialStrs[special_instruction], instruction, sSpecialInstrInfo[special_instruction].format_str, (u32)pc);
                }
                break;

                case FAULT_INST_OPCODE_REGIMM: 
                {
                    u32 regimm_instruction = (instruction >> FAULT_INST_REGIMM_SHIFT) & FAULT_INST_REGIMM_MASK;
                    strcpy(instruction_str, sRegimmStrs[regimm_instruction]);
                } 
                break;

                case FAULT_INST_OPCODE_COP0:
                case FAULT_INST_OPCODE_COP1:
                case FAULT_INST_OPCODE_COP2:
                {
                    if(instruction & FAULT_INST_CO_FLAG)
                    {
                        u32 cp0_instruction = (instruction >> FAULT_INST_CP0_SHIFT) & FAULT_INST_CP0_MASK;
                        strcpy(instruction_str, sCp0Strs[cp0_instruction]);
                    }
                    else
                    {
                        // u32 cop_instruction = ((instruction >> 13) & 0x7) | ((instruction << 3) & 0x18);
                        u32 cop_instruction = (instruction >> FAULT_INST_COPZRS_SHIFT) & FAULT_INST_COPZRS_MASK;

                        if(cop_instruction == FAULT_INST_COPZRS_BC)
                        {
                            u32 bc_instruction = (instruction >> FAULT_INST_COPZRT_SHIFT) & FAULT_INST_COPZRT_MASK;
                            strcpy(instruction_str, sCopzrtStrs[bc_instruction]);
                        }
                        else
                        {
                            strcpy(instruction_str, sCopzrsStrs[cop_instruction]);
                        }
                    }
                }
                break;

                default:
                    Fault_InstructionStr(instruction_str, sOpcodeStrs[opcode], instruction, sOpcodeInfos[opcode].format_str, (u32)pc);
                break;
            }

            FaultDrawer_SetCharPad(-2, 0);
            FaultDrawer_Printf("%08x", pc + line);

            FaultDrawer_SetCharPad(-1, 0);
            FaultDrawer_Printf("|");
            
            FaultDrawer_SetCharPad(-2, 0);
            FaultDrawer_Printf("%08x", instruction);

            FaultDrawer_SetCharPad(-1, 0);
            FaultDrawer_Printf("|");
            FaultDrawer_Printf("%s%s\n", ((u32)(pc + line) == thread->context.pc) ? "*" : " ", instruction_str);

            // FaultDrawer_SetCharPad(-2, 0);
            // FaultDrawer_Printf("%08x | %08x |", pc + line, instruction);
            // FaultDrawer_SetCharPad(-1, 0);
            // FaultDrawer_Printf("%s%s\n", ((u32)(pc + line) == thread->context.pc) ? "*" : " ", instruction_str);
            // FaultDrawer_Printf("%08x | %08x |%s%s\n", pc + line, instruction, ((u32)(pc + line) == thread->context.pc) ? "*" : " ", instruction_str);
        }

        button = Fault_WaitForInput2();

        if(CHECK_BTN_ANY(button, BTN_L | BTN_R))
        {
            return button;
        }

        multiplier = 1;


        if(CHECK_BTN_ALL(button, BTN_A | BTN_B))
        {
            multiplier = 1024;
        }
        else if(CHECK_BTN_ALL(button, BTN_A))
        {
            multiplier = 4;
        }
        else if(CHECK_BTN_ALL(button, BTN_B))
        {
            multiplier = 64;
        }
        

        if(CHECK_BTN_ANY(button, BTN_DUP))
        {
            pc -= multiplier;
        }
        else if(CHECK_BTN_ANY(button, BTN_DDOWN))
        {
            pc += multiplier;
        }
    }
    while(true);
}

/**
 * Searches a single function's stack frame for the function it was called from.
 * There are two cases that must be covered: Leaf and non-leaf functions.
 *
 * A leaf function is one that does not call any other function, in this case the
 * return address need not be saved to the stack. Since a leaf function does not
 * call other functions, only the function the stack trace begins in could possibly
 * be a leaf function, in which case the return address is in the thread context's
 * $ra already, as it never left.
 *
 * The procedure is therefore
 *  - Iterate instructions
 *  - Once jr $ra is found, set pc to $ra
 *  - Done after delay slot
 *
 * A non-leaf function calls other functions, it is necessary for the return address
 * to be saved to the stack. In these functions, it is important to keep track of the
 * stack frame size of each function.
 *
 * The procedure is therefore
 *  - Iterate instructions
 *  - If lw $ra <imm>($sp) is found, fetch the saved $ra from stack memory
 *  - If addiu $sp, $sp, <imm> is found, modify $sp by the immediate value
 *  - If jr $ra is found, set pc to $ra
 *  - Done after delay slot
 *
 * Note that searching for one jr $ra is sufficient, as only leaf functions can have
 * multiple jr $ra in the same function.
 *
 * There is also additional handling for eret and j. Neither of these instructions
 * appear in IDO compiled C, however do show up in the exception handler. It is not
 * possible to backtrace through an eret as an interrupt can occur at any time, so
 * there is no choice but to give up here. For j instructions, they can be followed
 * and the backtrace may continue as normal.
 */
// void Fault_WalkStack(uintptr_t* spPtr, uintptr_t* pcPtr, uintptr_t* raPtr) {
//     uintptr_t sp = *spPtr;
//     uintptr_t pc = *pcPtr;
//     uintptr_t ra = *raPtr;
//     u32 lastInsn;
//     u16 insnHi;
//     s16 insnLo;
//     u32 imm;

//     if ((sp % 4 != 0) || (sp < K0BASE) || (sp >= K2BASE) || (ra % 4 != 0) || (ra < K0BASE) || (ra >= K2BASE)) {
//         *spPtr = 0;
//         *pcPtr = 0;
//         *raPtr = 0;
//         return;
//     }

//     if ((pc % 4 != 0) || (pc < K0BASE) || (pc >= K2BASE)) {
//         *pcPtr = ra;
//         return;
//     }

//     lastInsn = 0;
//     while (true) {
//         insnHi = *(uintptr_t*)pc >> 16;
//         insnLo = *(uintptr_t*)pc & 0xFFFF;
//         imm = insnLo;

//         if (insnHi == 0x8FBF) {
//             // lw $ra, <imm>($sp)
//             // read return address saved on the stack
//             ra = *(uintptr_t*)(sp + imm);
//         } else if (insnHi == 0x27BD) {
//             // addiu $sp, $sp, <imm>
//             // stack pointer increment or decrement
//             sp += imm;
//         } else if (*(uintptr_t*)pc == 0x42000018) {
//             // eret
//             // cannot backtrace through an eret, give up
//             sp = 0;
//             pc = 0;
//             ra = 0;
//             goto done;
//         }
//         if (lastInsn == 0x3E00008) {
//             // jr $ra
//             // return to previous function
//             pc = ra;
//             goto done;
//         } else if (lastInsn >> 26 == 2) {
//             // j <target>
//             // extract jump target
//             pc = (pc >> 28 << 28) | (lastInsn << 6 >> 4);
//             goto done;
//         }

//         lastInsn = *(uintptr_t*)pc;
//         pc += sizeof(u32);
//     }

// done:
//     *spPtr = sp;
//     *pcPtr = pc;
//     *raPtr = ra;
// }

// /**
//  * Draws the stack trace page contents for the specified thread
//  */
// void Fault_DrawStackTrace(OSThread* thread, u32 flags) {
//     s32 line;
//     uintptr_t sp = thread->context.sp;
//     uintptr_t ra = thread->context.ra;
//     uintptr_t pc = thread->context.pc;
//     s32 pad;
//     uintptr_t addr;

//     Fault_FillScreenBlack();
//     FaultDrawer_DrawText(120, 16, "STACK TRACE");
//     FaultDrawer_DrawText(36, 24, "SP       PC       (VPC)");

//     for (line = 1; (line < 22) && (((ra != 0) || (sp != 0)) && (pc != (uintptr_t)__osCleanupThread)); line++) {
//         FaultDrawer_DrawText(36, line * 8 + 24, "%08x %08x", sp, pc);

//         if (flags & 1) {
//             // Try to convert the relocated program counter to the corresponding unrelocated virtual address
//             addr = Fault_ConvertAddress(pc);
//             if (addr != 0) {
//                 FaultDrawer_Printf(" -> %08x", addr);
//             }
//         } else {
//             FaultDrawer_Printf(" -> ????????");
//         }

//         Fault_WalkStack(&sp, &pc, &ra);
//     }
// }

u32 Fault_WalkStack2(uintptr_t *cur_pc, uintptr_t *cur_sp, uintptr_t *cur_ra, struct FaultStackTraceInfo *trace_info)
{
    uintptr_t pc = *cur_pc;
    uintptr_t sp = *cur_sp;
    uintptr_t ra = *cur_ra;
    uintptr_t start_pc = pc;
    u32 can_continue = true;
    u32 delay_slot_counter = 0;
    u32 ret_addr = 0;

    if ((sp % 4 != 0) || (sp < K0BASE) || (sp >= K2BASE) || (ra % 4 != 0) || (ra < K0BASE) || (ra >= K2BASE)) 
    {
        *cur_sp = 0;
        *cur_pc = 0;
        *cur_ra = 0;
        return false;
    }

    if ((pc % 4 != 0) || (pc < K0BASE) || (pc >= K2BASE)) 
    {
        *cur_pc = ra;
        return false;
    }

    while(true)
    {
        u32 instruction = *(u32 *)pc;
        u32 opcode = (instruction >> FAULT_INST_OPCODE_SHIFT);

        switch(opcode)
        {
            case FAULT_INST_OPCODE_SPECIAL:
            {
                u32 special_instruction = instruction & FAULT_INST_SPECIAL_MASK;
                
                if(special_instruction == FAULT_INST_SPECIAL_JR)
                {
                    u32 src_reg = FAULT_INST_RS_RT_RD_SA_GET_RS(instruction);

                    if(src_reg == FAULT_CPU_REG_RA)
                    {
                        ret_addr = ra;
                        trace_info->pc = ret_addr - 8;
                        delay_slot_counter = 2;
                    }
                }
            }
            break;

            case FAULT_INST_OPCODE_COP0:
            {
                u32 cop0_instruction = instruction & 0x3f;

                if(cop0_instruction == FAULT_INST_CP0_ERET)
                {
                    can_continue = false;
                    goto done;
                }
            }
            break;

            case FAULT_INST_OPCODE_J:
            {
                ret_addr = (pc & 0xf0000000) | FAULT_INST_TARGET_GET_TARGET(instruction);
                delay_slot_counter = 2;
            }
            break;

            case FAULT_INST_OPCODE_ADDIU:
            case FAULT_INST_OPCODE_ADDI:
            {
                u32 src_reg = FAULT_INST_RS_RT_IMM_GET_RS(instruction);
                u32 dst_reg = FAULT_INST_RS_RT_IMM_GET_RT(instruction);

                if(src_reg == dst_reg && dst_reg == FAULT_CPU_REG_SP)
                {
                    u32 imm = FAULT_INST_RS_RT_IMM_GET_IMM(instruction);
                    if(imm & 0x8000)
                    {
                        imm |= 0xffff0000;
                    }
                    trace_info->sp = sp;
                    sp += imm;
                }
            }
            break;

            case FAULT_INST_OPCODE_LW:
            {
                u32 dst_reg = FAULT_INST_BASE_RT_OFFSET_GET_RT(instruction);
                u32 base_reg = FAULT_INST_BASE_RT_OFFSET_GET_BASE(instruction);

                if(dst_reg == FAULT_CPU_REG_RA && base_reg == FAULT_CPU_REG_SP)
                {
                    u32 offset = FAULT_INST_BASE_RT_OFFSET_GET_OFFSET(instruction);

                    if(offset & 0x8000)
                    {
                        offset |= 0xffff0000;
                    }

                    ra = *(uintptr_t *)(sp + offset);
                }
            }
            break;
        }

        if(delay_slot_counter > 0)
        {
            delay_slot_counter--;

            if(delay_slot_counter == 0)
            {
                pc = ret_addr;
                goto done;
            }
        }

        pc += 4;
    }

    done:
    *cur_pc = pc;
    *cur_sp = sp;
    *cur_ra = ra;

    // if(can_continue)
    // {
    //     u32 instruction;
    //     u32 opcode;

    //     /* try to figure out the function address */

    //     pc = trace_info->call_site;
    //     instruction = *(u32 *)pc;
    //     opcode = instruction >> FAULT_INST_OPCODE_SHIFT;

    //     switch(opcode)
    //     {
    //         // case FAULT_INST_OPCODE_JAL:
    //         // case FAULT_INST_OPCODE_J:
    //         // {
    //         //     u32 delay_address_pc = trace_info->call_site + 4;
    //         //     /* call site used jal, so function address is directly available */
    //         //     trace_info->func_start = (delay_address_pc & 0xf0000000) | FAULT_INST_TARGET_GET_TARGET(instruction);
    //         // }
    //         // break;

    //         default:
    //             /* something else got used, so try walking back until we hit an addiu sp, sp, imm  */
    //             while(true)
    //             {
    //                 u32 instruction = *(u32 *)pc;
    //                 u32 opcode = instruction >> FAULT_INST_OPCODE_SHIFT;

    //                 if(opcode == FAULT_INST_OPCODE_ADDI || opcode == FAULT_INST_OPCODE_ADDIU)
    //                 {
    //                     u32 src_reg = FAULT_INST_RS_RT_IMM_GET_RS(instruction);
    //                     u32 dst_reg = FAULT_INST_RS_RT_IMM_GET_RT(instruction);

    //                     if(src_reg == dst_reg && dst_reg == FAULT_CPU_REG_SP)
    //                     {
    //                         trace_info->func_start = pc;
    //                         break;
    //                     }
    //                 }

    //                 pc -= 4;
    //             }
    //         break;
    //     }
    // }
    return can_continue;
}

void Fault_FindCaleeAddress(struct FaultStackTraceInfo *trace_info)
{
    u32 instruction;
    u32 opcode;

    /* try to figure out the function address */

    u32 pc = trace_info->pc;
    instruction = *(u32 *)pc;
    opcode = instruction >> FAULT_INST_OPCODE_SHIFT;

    switch(opcode)
    {
        default:
            /* something else got used, so try walking back until we hit an addiu sp, sp, imm  */
            while(true)
            {
                u32 instruction = *(u32 *)pc;
                u32 opcode = instruction >> FAULT_INST_OPCODE_SHIFT;

                if(opcode == FAULT_INST_OPCODE_ADDI || opcode == FAULT_INST_OPCODE_ADDIU)
                {
                    u32 src_reg = FAULT_INST_RS_RT_IMM_GET_RS(instruction);
                    u32 dst_reg = FAULT_INST_RS_RT_IMM_GET_RT(instruction);

                    if(src_reg == dst_reg && dst_reg == FAULT_CPU_REG_SP)
                    {
                        trace_info->func = pc;
                        break;
                    }
                }

                pc -= 4;
            }
        break;
    }
}

u32 Fault_DrawStackTrace2(OSThread* thread, u32 flags) {
    s32 line;
    uintptr_t sp = thread->context.sp;
    uintptr_t ra = thread->context.ra;
    uintptr_t pc = thread->context.pc;
    s32 pad;
    uintptr_t addr;
    struct FaultStackTraceInfo trace_info;

    Fault_FillScreenBlack();
    FaultDrawer_DrawText(120, 16, "STACK TRACE");
    FaultDrawer_DrawText(36, 24, "  FUNC  :  PC");
    FaultDrawer_SetCharPad(-1, 0);
    trace_info.pc = pc;
    trace_info.sp = sp;
    trace_info.func = 0;

    for(line = 1; line < 22; line++)
    {
        uintptr_t real_func_addr;
        // FaultDrawer_DrawText(36, line * 8 + 24, "%08x %08x", sp, pc);

        // u32 can_continue = Fault_WalkStack2(&pc, &sp, &ra, &trace_info);
        Fault_FindCaleeAddress(&trace_info);
        
        real_func_addr = Fault_ConvertAddress(trace_info.func);

        if(real_func_addr != 0)
        {
            uintptr_t offset = real_func_addr - trace_info.func;
            trace_info.pc += offset;
            trace_info.func = real_func_addr;
        }

        FaultDrawer_DrawText(36, line * 8 + 24, "%08x:%08x", (u32)trace_info.func, (u32)trace_info.pc);
        if(!Fault_WalkStack2(&pc, &sp, &ra, &trace_info))
        {
            break;
        }
    }

    return Fault_WaitForInput2();

    // for (line = 1; (line < 22) && (((ra != 0) || (sp != 0)) && (pc != (uintptr_t)__osCleanupThread)); line++) {
    //     FaultDrawer_DrawText(36, line * 8 + 24, "%08x %08x", sp, pc);

    //     if (flags & 1) {
    //         // Try to convert the relocated program counter to the corresponding unrelocated virtual address
    //         addr = Fault_ConvertAddress(pc);
    //         if (addr != 0) {
    //             FaultDrawer_Printf(" -> %08x", addr);
    //         }
    //     } else {
    //         FaultDrawer_Printf(" -> ????????");
    //     }

    //     Fault_WalkStack(&sp, &pc, &ra);
    // }
}

// void Fault_LogStackTrace(OSThread* thread, u32 flags) {
//     s32 line;
//     uintptr_t sp = thread->context.sp;
//     uintptr_t ra = thread->context.ra;
//     uintptr_t pc = thread->context.pc;
//     uintptr_t addr;

//     osSyncPrintf("STACK TRACE");
//     osSyncPrintf("SP       PC       (VPC)\n");

//     for (line = 1; (line < 22) && (((ra != 0) || (sp != 0)) && (pc != (uintptr_t)__osCleanupThread)); line++) {
//         osSyncPrintf("%08x %08x", sp, pc);

//         if (flags & 1) {
//             // Try to convert the relocated program counter to the corresponding unrelocated virtual address
//             addr = Fault_ConvertAddress(pc);
//             if (addr != 0) {
//                 osSyncPrintf(" -> %08x", addr);
//             }
//         } else {
//             osSyncPrintf(" -> ????????");
//         }
//         osSyncPrintf("\n");

//         Fault_WalkStack(&sp, &pc, &ra);
//     }
// }

void Fault_ResumeThread(OSThread* thread) {
    thread->context.cause = 0;
    thread->context.fpcsr = 0;
    thread->context.pc += sizeof(u32);
    *(u32*)thread->context.pc = 0x0000000D; // write in a break instruction
    osWritebackDCache((void*)thread->context.pc, 4);
    osInvalICache((void*)thread->context.pc, 4);
    osStartThread(thread);
}

void Fault_DisplayFrameBuffer(void) {
    void* fb;

    osViSetYScale(1.0f);
    osViSetMode(&osViModeNtscLan1);
    osViSetSpecialFeatures(OS_VI_GAMMA_OFF | OS_VI_DITHER_FILTER_ON);
    osViBlack(false);

    if (sFaultInstance->fb) {
        fb = sFaultInstance->fb;
    } else {
        fb = osViGetNextFramebuffer();
        if ((uintptr_t)fb == K0BASE) {
            fb = (void*)(PHYS_TO_K0(osMemSize) - SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(u16));
        }
    }

    osViSwapBuffer(fb);
    FaultDrawer_SetDrawerFrameBuffer(fb, SCREEN_WIDTH, SCREEN_HEIGHT);
}

/**
 * Runs all registered fault clients. Each fault client displays a page
 * on the crash screen.
 */
u32 Fault_ProcessClients(u32 button) {
    // FaultClient* client = sFaultInstance->clients;
    FaultClient *client = NULL;
    s32 client_index = 0;
    u32 client_count = 0;

    client = sFaultInstance->first_client;
    while(client != NULL)
    {
        client_count++;
        client = client->next;
    }
    // u32 last_page_bitmask = 0;

    // if(just_changed)
    // {
    //     if(button == BTN_L)
    //     {
    //         sFaultInstance->current_displayed_client = sFaultInstance->last_client;
    //     }
    //     else if(button == BTN_R)
    //     {
    //         sFaultInstance->current_displayed_client = sFaultInstance->first_client;
    //     }
    // }
    // else
    // {
    //     if(button == BTN_L)
    //     {
    //         sFaultInstance->current_displayed_client = sFaultInstance->current_displayed_client->prev;
    //         sFaultInstance->current_displayed_client_index--;
    //         // idx--;
    //     }
    //     else if(button == BTN_R)
    //     {
    //         sFaultInstance->current_displayed_client = sFaultInstance->current_displayed_client->next;
    //         sFaultInstance->current_displayed_client_index++;
    //         // idx++;
    //     }

    // }

    if(button == BTN_L)
    {
        client = sFaultInstance->last_client;
        client_index = client_count - 1;
    }
    else if(button == BTN_R)
    {
        client = sFaultInstance->first_client;
        client_index = 0;
    }

    // while (client != NULL) {

    while(true)
    {

        if(client != NULL)
        {
            if (client->callback != NULL) 
            {
                FaultDrawer_FillScreen();
                FaultDrawer_SetCharPad(-2, 0);
                FaultDrawer_Printf(FAULT_COLOR(DARK_GRAY) "CallBack (%d) %08x %08x %08x\n" FAULT_COLOR(WHITE), client_index, client, client->arg0, client->arg1);

                FaultDrawer_SetCharPad(0, 0);
                client->callback(client->arg0, client->arg1);
            }

            button = Fault_WaitForInput2();

            if(button == BTN_L)
            {
                client = client->prev;
                client_index--;
            }
            else if(button == BTN_R)
            {
                client = client->next;
                client_index++;
            }
        }
        else
        {
            break;
        }
    }

    return button;

    // return last_page_bitmask;
    
    // return true;
}

void Fault_SetOptions(void) {
    static u32 sFaultCustomOptions;
    Input* input3 = &sFaultInstance->inputs[3];
    s32 pad;
    uintptr_t pc;
    uintptr_t ra;
    uintptr_t sp;

    if (CHECK_BTN_ALL(input3->press.button, BTN_RESET)) {
        sFaultCustomOptions = !sFaultCustomOptions;
    }

    if (sFaultCustomOptions) {
        pc = gGraphThread.context.pc;
        ra = gGraphThread.context.ra;
        sp = gGraphThread.context.sp;
        if (CHECK_BTN_ALL(input3->cur.button, BTN_R)) {
            static u32 sFaultCopyToLog;

            sFaultCopyToLog = !sFaultCopyToLog;
            FaultDrawer_SetOsSyncPrintfEnabled(sFaultCopyToLog);
        }
        if (CHECK_BTN_ALL(input3->cur.button, BTN_A)) {
            osSyncPrintf("GRAPH PC=%08x RA=%08x STACK=%08x\n", pc, ra, sp);
        }
        if (CHECK_BTN_ALL(input3->cur.button, BTN_B)) {
            FaultDrawer_SetDrawerFrameBuffer(osViGetNextFramebuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
            Fault_DrawRec(0, 215, 320, 9, 1);
            FaultDrawer_SetCharPad(-2, 0);
            FaultDrawer_DrawText(32, 216, "GRAPH PC %08x RA %08x SP %08x", pc, ra, sp);
        }
    }
}

void Fault_UpdatePad(void) {
    Fault_UpdatePadImpl();
    Fault_SetOptions();
}

#define FAULT_MSG_CPU_BREAK ((OSMesg)1)
#define FAULT_MSG_FAULT ((OSMesg)2)
#define FAULT_MSG_UNK ((OSMesg)3)
#define FAULT_MSG_SW1 ((OSMesg)4)


void Fault_ThreadEntry(void* arg) {
    OSMesg msg;
    u32 pad;
    OSThread* faultedThread;
    Input* input;
    u32 page_index;
    u32 next_page_index;
    u32 prev_page_index;
    u32 button;
    u32 last_page_bitmask = 0;
    bool just_changed_page = false;
    bool skip_input_pooling = true;

    // Direct OS event messages to the fault event queue
    osSetEventMesg(OS_EVENT_CPU_BREAK, &sFaultInstance->queue, FAULT_MSG_CPU_BREAK);
    osSetEventMesg(OS_EVENT_FAULT, &sFaultInstance->queue, FAULT_MSG_FAULT);
    osSetEventMesg(OS_EVENT_SW1, &sFaultInstance->queue, FAULT_MSG_SW1);

    while (true) {
        do {
            // Wait for a thread to hit a fault
            osRecvMesg(&sFaultInstance->queue, &msg, OS_MESG_BLOCK);

            if (msg == FAULT_MSG_CPU_BREAK) {
                sFaultInstance->msgId = (u32)FAULT_MSG_CPU_BREAK;
                // "Fault manager: OS_EVENT_CPU_BREAK received"
                osSyncPrintf("フォルトマネージャ:OS_EVENT_CPU_BREAKを受信しました\n");
            } else if (msg == FAULT_MSG_FAULT) {
                sFaultInstance->msgId = (u32)FAULT_MSG_FAULT;
                // "Fault manager: OS_EVENT_FAULT received"
                osSyncPrintf("フォルトマネージャ:OS_EVENT_FAULTを受信しました\n");
            } else if (msg == FAULT_MSG_UNK) {
                Fault_UpdatePad();
                faultedThread = NULL;
                continue;
            } 
            else if (msg == FAULT_MSG_SW1)
            {
                FaultDrawer_FillScreen();
                FaultDrawer_SetCharPad(-2, 4);
                FaultDrawer_SetCursor(22, 20);
                FaultDrawer_Printf("SHIT'S FUCKED ");
                Fault_DisplayFrameBuffer();
                while(true){}
            }
            else {
                sFaultInstance->msgId = (u32)FAULT_MSG_UNK;
                // "Fault manager: received an unknown message"
                osSyncPrintf("フォルトマネージャ:不明なメッセージを受信しました\n");
            }

            faultedThread = __osGetCurrFaultedThread();
            osSyncPrintf("__osGetCurrFaultedThread()=%08x\n", faultedThread);

            if (faultedThread == NULL) {
                faultedThread = Fault_FindFaultedThread();
                osSyncPrintf("FindFaultedThread()=%08x\n", faultedThread);
            }
        } while (faultedThread == NULL);


        // if(msg == FAULT_MSG_FAULT)
        {
            __osSetFpcCsr(__osGetFpcCsr() & ~(FPCSR_EV | FPCSR_EZ | FPCSR_EO | FPCSR_EU | FPCSR_EI));
            sFaultInstance->faultedThread = faultedThread;

            while (!sFaultInstance->faultHandlerEnabled) {
                Fault_Sleep(1000);
            }
            Fault_Sleep(1000 / 2);

            // Show fault framebuffer
            Fault_DisplayFrameBuffer();

            if(gCurrentGameState != NULL && gCurrentGameState->main == Play_Main && 
                Chaos_GetConfigFlag(CHAOS_CONFIG_SAVE_AT_GAME_CRASH) &&
                !gChaosContext.fake_crash)
            {
                Sram_SaveAtGameCrashTime((struct PlayState *)gCurrentGameState);
            }

            // if (sFaultInstance->autoScroll) {
            //     Fault_Wait5Seconds();
            // } else {
            //     // Draw error bar signifying the crash screen is available
            //     Fault_DrawCornerRec(GPACK_RGBA5551(255, 0, 0, 1));
            //     // Fault_WaitForButtonCombo();
            // }

            // Set auto-scrolling and default colors
            // sFaultInstance->autoScroll = true;
            // FaultDrawer_SetForeColor(GPACK_RGBA5551(255, 255, 255, 1));
            // FaultDrawer_SetBackColor(GPACK_RGBA5551(0, 0, 0, 0));


            // sFaultInstance->current_disasm_pc = faultedThread->context.pc;
            // Draw pages

            if(gChaosContext.fake_crash)
            {
                OSMesgQueue timer_queue;
                OSMesg timer_message;
                OSTimer timer;
                u32 timeout = Chaos_RandS16Offset(1200, 1800);
                u32 return_address = faultedThread->context.pc;
                faultedThread->context.pc = 0x80000000 + ((Chaos_RandNext() % 0xffffff) & 0xfffffc);
                FaultDrawer_SetForeColor(GPACK_RGBA5551(255, 255, 255, 1));
                FaultDrawer_SetBackColor(GPACK_RGBA5551(0, 0, 0, 0));
                Fault_PrintThreadContextInfo(faultedThread);
                faultedThread->context.pc = return_address;
                gChaosContext.fake_crash_pointer = &gChaosContext.fake_crash;
                osCreateMesgQueue(&timer_queue, &timer_message, 1);
                osSetTimer(&timer, OS_USEC_TO_CYCLES(timeout * 1000), 0, &timer_queue, (OSMesg)0);
                osRecvMesg(&timer_queue, NULL, OS_MESG_BLOCK);
            }
            else
            {
                input = &sFaultInstance->inputs[0];
                page_index = 0;
                prev_page_index = 0xffffffff;
                just_changed_page = true;
                button = 0;

                do {
                    // Fault_FillScreenBlack();
                    FaultDrawer_SetForeColor(GPACK_RGBA5551(255, 255, 255, 1));
                    FaultDrawer_SetBackColor(GPACK_RGBA5551(0, 0, 0, 0));
                    // Fault_DisplayFrameBuffer()

                    switch(page_index)
                    {
                        case 0:
                            button = Fault_PrintThreadContext(faultedThread);
                        break;

                        case 1:
                            // Fault_DrawStackTrace(faultedThread, 0);
                            // Fault_LogStackTrace(faultedThread, 0);
                            button = Fault_DrawStackTrace2(faultedThread, 0);
                        break;

                        case 2:
                            button = Fault_Disasm(faultedThread, button);
                        break;

                        case 3:
                            button = Fault_ProcessClients(button);
                        break;    

                        case 4:
                            button = Fault_DrawMemDump(faultedThread, button);
                        break;

                        // case 5:
                        //     Fault_DrawStackTrace(faultedThread, 1);
                        //     Fault_LogStackTrace(faultedThread, 1);
                        // break;
                    }

                    if(button == 0)
                    {
                        button = Fault_WaitForInput2();
                    }

                    if(button == BTN_L)
                    {
                        page_index = (page_index - 1) % 0xfffffffb;
                    }
                    else if(button == BTN_R)
                    {
                        page_index = (page_index + 1) % 5;
                    }
                } while (true);
            }
        }

        Fault_ResumeThread(faultedThread);
    }
}

void Fault_SetFrameBuffer(void* fb, u16 w, u16 h) {
    sFaultInstance->fb = fb;
    FaultDrawer_SetDrawerFrameBuffer(fb, w, h);
}

STACK(sFaultStack, 0x600);
StackEntry sFaultStackInfo;
FaultMgr gFaultMgr;

void Fault_Init(void) {
    sFaultInstance = &gFaultMgr;
    bzero(sFaultInstance, sizeof(FaultMgr));
    FaultDrawer_Init();
    FaultDrawer_SetInputCallback(Fault_WaitForInput);
    sFaultInstance->exit = false;
    sFaultInstance->msgId = 0;
    sFaultInstance->faultHandlerEnabled = false;
    sFaultInstance->faultedThread = NULL;
    sFaultInstance->padCallback = Fault_PadCallback;
    sFaultInstance->first_client = NULL;
    sFaultInstance->last_client = NULL;
    sFaultInstance->autoScroll = false;
    gFaultMgr.faultHandlerEnabled = true;
    osCreateMesgQueue(&sFaultInstance->queue, sFaultInstance->msg, ARRAY_COUNT(sFaultInstance->msg));
    StackCheck_Init(&sFaultStackInfo, sFaultStack, STACK_TOP(sFaultStack), 0, 0x100, "fault");
    osCreateThread(&sFaultInstance->thread, Z_THREAD_ID_FAULT, Fault_ThreadEntry, NULL, STACK_TOP(sFaultStack),
                   Z_PRIORITY_FAULT);
    osStartThread(&sFaultInstance->thread);
}

/**
 * Fault page for Hungup crashes. Displays the thread id and two messages
 * specified in arguments to `Fault_AddHungupAndCrashImpl`.
 */
void Fault_HangupFaultClient(const char* exp1, const char* exp2) {
    osSyncPrintf("HungUp on Thread %d\n", osGetThreadId(NULL));
    osSyncPrintf("%s\n", exp1 != NULL ? exp1 : "(NULL)");
    osSyncPrintf("%s\n", exp2 != NULL ? exp2 : "(NULL)");
    FaultDrawer_Printf("HungUp on Thread %d\n", osGetThreadId(NULL));
    FaultDrawer_Printf("%s\n", exp1 != NULL ? exp1 : "(NULL)");
    FaultDrawer_Printf("%s\n", exp2 != NULL ? exp2 : "(NULL)");
}

/**
 * Immediately crashes the current thread, for cases where an irrecoverable
 * error occurs. The parameters specify two messages detailing the error, one
 * or both may be NULL.
 */
void Fault_AddHungupAndCrashImpl(const char* exp1, const char* exp2) {
    FaultClient client;
    s32 pad;

    Fault_AddClient(&client, (void*)Fault_HangupFaultClient, (void*)exp1, (void*)exp2);
    *(u32*)0x11111111 = 0; // trigger an exception via unaligned memory access

    // Since the above line triggers an exception and transfers execution to the fault handler
    // this function does not return and the rest of the function is unreachable.
    UNREACHABLE();
}

/**
 * Like `Fault_AddHungupAndCrashImpl`, however provides a fixed message containing
 * file and line number
 */
NORETURN void Fault_AddHungupAndCrash(const char* file, s32 line) {
    char msg[0x100];

    sprintf(msg, "HungUp %s:%d", file, line);
    Fault_AddHungupAndCrashImpl(msg, NULL);
}

struct FaultHangupPrintfClientParams
{
    const char *format;
    va_list     args;
};

void Fault_HangupFaultPrintfClient(void *args, void *unused)
{
    struct FaultHangupPrintfClientParams *params = (struct FaultHangupPrintfClientParams *)args;
    FaultDrawer_Printf("HungUp on Thread %d\n", osGetThreadId(NULL));
    FaultDrawer_VPrintf(params->format, params->args);
}

void Fault_AddHangupPrintfAndCrash(const char *fmt, ...)
{
    struct FaultHangupPrintfClientParams params;
    FaultClient client;

    params.format = fmt;
    va_start(params.args, fmt);

    Fault_AddClient(&client, (void*)Fault_HangupFaultPrintfClient, (void*)&params, NULL);
    *(u32*)0x11111111 = 0; // trigger an exception via unaligned memory access
}
