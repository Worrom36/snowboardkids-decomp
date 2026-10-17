#ifndef _OS_VI_H_
#define _OS_VI_H_
#include <PR/ultratypes.h>
#include <PR/os_thread.h>
#include <PR/os_message.h>
typedef struct { u32 ctrl; u32 width; u32 burst; u32 vSync; u32 hSync; u32 leap; u32 hStart; u32 xScale; u32 vCurrent; } OSViCommonRegs;
typedef struct { u32 origin; u32 yScale; u32 vStart; u32 vBurst; u32 vIntr; } OSViFieldRegs;
typedef struct { u8 type; OSViCommonRegs comRegs; OSViFieldRegs fldRegs[2]; } OSViMode;
extern void osCreateViManager(OSPri);
extern void osViSetMode(OSViMode *);
extern void osViBlack(u8);
extern void osViSetEvent(OSMesgQueue *, OSMesg, u32);
extern void *osViGetCurrentFramebuffer(void);
extern void *osViGetNextFramebuffer(void);
extern void osViSwapBuffer(void *);
#endif
