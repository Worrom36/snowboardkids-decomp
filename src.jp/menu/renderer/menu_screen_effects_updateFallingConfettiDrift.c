#include "game/race/ui/race_ui_effects.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
#include "common.h"
#include "game/audio/sound_manager.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/asset_manager.h"
#include "game/engine/system_runtime.h"
#include "game/engine/viewport_manager.h"
#include "game/math/fixed_point_math.h"
#include "game/menu/renderer/menu_screen_effects.h"
#include "game/menu/main_menu/main_menu_scene_model.h"
#include "game/menu/main_menu/main_menu_scene_model_renderer.h"
#include "game/race/player/race_player_input.h"

extern void drawFallingConfetti(MenuScreenEffectActor *arg0);
extern void updateFallingConfettiSway(MenuScreenEffectActor *arg0);
extern void updateFallingConfettiDrift(MenuScreenEffectActor *arg0);
extern u32 D_20006C8[];
extern u32 D_2000000[];
extern u32 D_2000D78[];
extern u32 D_20058A8[];
extern void drawPassAwardPodiumModels(MenuScreenEffectActor *);
extern void drawPassAwardCourseBackdrop(void *);
extern void renderPassAwardPodium(MenuScreenEffectActor *);

void updateFallingConfettiDrift(MenuScreenEffectActor *arg0) {
    s16 temp_v0;
    s16 temp_v0_2;
    u32 var_v1;

    arg0->unk1C.half.hi = (arg0->unk1C.half.hi + 1) & 3;
    if (arg0->unk24.half.hi >= 0) {
        temp_v0 = arg0->unk24.half.lo;
        if (temp_v0 != 0) {
            arg0->unk20.half.hi += 4;
            arg0->unk24.half.lo = temp_v0 - 1;
        } else {
            arg0->unk20.half.hi -= 4;
            if (arg0->unk20.half.hi == 0) {
                arg0->unk24.half.lo = (randomNextMain() & 0x1F) + 0x10;
                arg0->unk24.half.hi = -1;
            }
        }
    } else {
        temp_v0_2 = arg0->unk24.half.lo;
        if (temp_v0_2 != 0) {
            arg0->unk20.half.hi -= 4;
            arg0->unk24.half.lo = temp_v0_2 - 1;
        } else {
            arg0->unk20.half.hi += 4;
            if (arg0->unk20.half.hi == 0) {
                arg0->unk24.half.lo = (randomNextMain() & 0x1F) + 0x10;
                arg0->unk24.half.hi = 1;
            }
        }
    }

    var_v1 = arg0->unk20.half.hi;
    arg0->unk18.half.hi += var_v1;
    arg0->unk18.half.lo += arg0->unk20.half.lo;
    if (arg0->unk18.half.lo >= 0x6E1) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gRaceOverlayRenderCallbackList, (RenderCallback)drawFallingConfetti, (void *)arg0);
}

void updateFallingConfettiSway(MenuScreenEffectActor *arg0) {
    arg0->unk1C.half.hi = (arg0->unk1C.half.hi + 1) & 3;
    arg0->unk18.half.lo += arg0->unk20.half.lo;
    arg0->unk24.half.hi = (arg0->unk24.half.hi + 0x20) & 0xFFF;
    arg0->unk18.half.hi += (fixedSine(arg0->unk24.half.hi) * (arg0->unk1C.half.lo + 0x30)) / 4096;
    if ((arg0->unk24.half.hi == 0) || (arg0->unk24.half.hi == 0x800)) {
        arg0->unk1C.half.lo = randomNextMain() & 0xF;
    }
    if (arg0->unk18.half.lo >= 0x6E1) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gRaceOverlayRenderCallbackList, (RenderCallback)drawFallingConfetti, (void *)arg0);
}

void initFallingConfetti(MenuScreenEffectActor *arg0) {
    s32 temp_v1;
    s16 rand;

    temp_v1 = randomNextMain() & 1;
    switch (temp_v1) {
        case 0:
            arg0->unk18.half.hi = (randomNextMain() - 0x80) << 4;
            arg0->unk18.half.lo = -0x780;
            arg0->unk1C.half.lo = randomNextMain() & 0xF;
            rand = randomNextMain();
            arg0->unk24.half.hi = rand << 4;
            if (randomNextMain() & 1) {
                arg0->timer = 0x20;
                arg0->unk20.half.lo = (randomNextMain() & 7) + 0x40;
            } else {
                arg0->timer = 0x1B;
                arg0->unk20.half.lo = (randomNextMain() & 7) + 0x20;
            }
            arg0->unk2A = randomNextMain() % 5;
            setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateFallingConfettiSway);
            return;
        case 1:
            arg0->unk18.half.hi = (randomNextMain() - 0x80) << 4;
            arg0->unk18.half.lo = -0x780;
            arg0->unk1C.half.lo = (randomNextMain() & 0x1F) - 0xF;
            rand = randomNextMain();
            arg0->unk24.half.hi = rand << 4;
            if (randomNextMain() & 1) {
                arg0->timer = 0x20;
                arg0->unk20.half.lo = (randomNextMain() & 7) + 0x3C;
            } else {
                arg0->timer = 0x1B;
                arg0->unk20.half.lo = (randomNextMain() & 7) + 0x22;
            }
            arg0->unk2A = randomNextMain() % 5;
            arg0->unk24.half.hi = (randomNextMain() & 1) - 1;
            if (arg0->unk24.half.hi > 0) {
                arg0->unk20.half.hi = (randomNextMain() * 8) & 0xF;
            } else {
                arg0->unk20.half.hi = (randomNextMain() * -8) & 0xF;
            }
            arg0->unk24.half.lo = randomNextMain() & 0xF;
            setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateFallingConfettiDrift);
            return;
    }
}

void drawPassAwardPodiumModels(MenuScreenEffectActor *arg0) {
    register MenuScreenEffectActor *actor = arg0;

    if (gCurrentViewportIndex == 0) {
        gDPPipeSync(gRegionAllocPtr++);

        gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(gAssetHandles[8]));
        gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(gAssetHandles[9]));

        gSPMatrix(gRegionAllocPtr++, &gIdentityMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        if (actor->index == 0) {
            gSPDisplayList(gRegionAllocPtr++, D_2000000);
        }
        if (actor->index == 1) {
            gSPDisplayList(gRegionAllocPtr++, D_20006C8);
        }
        if (actor->index == 2) {
            gSPDisplayList(gRegionAllocPtr++, D_2000D78);
        }
    }
}

void drawPassAwardCourseBackdrop(void *arg0) {
    if (gCurrentViewportIndex == 0) {
        gDPPipeSync(gRegionAllocPtr++);

        gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(gAssetHandles[14]));
        gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(gAssetHandles[18]));

        gSPMatrix(gRegionAllocPtr++, &gIdentityMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        gSPDisplayList(gRegionAllocPtr++, D_20058A8);
    }
}

void renderPassAwardPodium(MenuScreenEffectActor *arg0) {
    addRenderCallback(&gBackdropRenderCallbackList, (RenderCallback)drawPassAwardCourseBackdrop, arg0);
    addRenderCallback(&gRaceObjectRenderCallbackList, (RenderCallback)drawPassAwardPodiumModels, arg0);
}

void initPassAwardPodium(MenuScreenEffectActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)renderPassAwardPodium);
}
