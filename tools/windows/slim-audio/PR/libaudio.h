#ifndef __LIB_AUDIO__
#define __LIB_AUDIO__
#include <PR/ultratypes.h>
#include <PR/mbi.h>
typedef s32 ALMicroTime;
typedef u8 ALPan;
typedef struct ALLink_s { struct ALLink_s *next; struct ALLink_s *prev; } ALLink;
typedef struct { ALLink *head; } ALHeap;
typedef struct ALPlayer_s { void *pad; } ALPlayer;
typedef struct ALVoice_s { void *pad; } ALVoice;
typedef struct ALWaveTable_s { void *pad; } ALWaveTable;
typedef struct ALSynth_s { void *pad; } ALSynth;
typedef struct {
    s32 maxVVoices; s32 maxPVoices; s32 maxUpdates; s32 maxFXbusses;
    void *dmaproc; ALHeap *heap; s32 outputRate; s32 fxType;
} ALSynConfig;
typedef void (*ALDMAproc)(void);
#endif
