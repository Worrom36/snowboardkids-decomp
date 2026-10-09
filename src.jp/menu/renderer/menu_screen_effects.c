#include "common.h"
#include "game/race/ui/race_ui_effects.h"
#include "game/engine/game_task_scheduler.h"
#include "game/race/race_state.h"
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

#define MAIN_MENU_GFX_CMD(pkt, cmd0, cmd1) \
    {                                      \
        Gfx *_g = (Gfx *)(pkt);            \
        _g->words.w0 = (cmd0);             \
        _g->words.w1 = (cmd1);             \
    }

typedef struct {
    Transform3D source;
    s32 pad20;
} TitleMenuRotatingBoardScratch;

extern s16 gMenuFadeAlpha;
extern s16 raceSetupCharacterFocusSoundIds[6];
extern u8 raceSetupCharacterFocusAnimationIds[8];
extern u8 passAwardOpponentAnimationIds[4];
extern s32 passAwardOpponentXPositions[6];
extern char D_800E0520[];
extern u32 D_20006C8[];
extern u32 D_2000000[];
extern u32 D_2000D78[];
extern u32 D_20058A8[];
extern Vtx raceStartPlayerEffectVertices[4];
extern s16 titleMenuSparklePositions[2];
extern u32 D_2000E48[];
extern u32 D_2000E70[];

void drawPassAwardLabel(MenuScreenEffectActor *);
void drawCourseRecordBanner(MenuScreenEffectActor *);

extern void drawFallingConfetti(MenuScreenEffectActor *arg0);
extern void updateFallingConfettiSway(MenuScreenEffectActor *arg0);
extern void updateFallingConfettiDrift(MenuScreenEffectActor *arg0);
extern void drawPassAwardPodiumModels(MenuScreenEffectActor *);
extern void drawPassAwardCourseBackdrop(void *);
extern void renderPassAwardPodium(MenuScreenEffectActor *);
extern void initPassAwardOpponent(MenuScreenEffectActor *arg0);
extern void drawTitleScreenStartPrompt(MenuScreenEffectActor *);
extern void drawTitleMenuSparkle(MenuScreenEffectActor *);
extern void renderTitleMenuSparkle(MenuScreenEffectActor *arg0);
extern void updateCourseRecordBannerFadeOut(MenuScreenEffectActor *);
extern void drawRaceSetupNamePlate(MenuScreenEffectActor *);
extern void updateRaceSetupNamePlateSlideOut(MenuScreenEffectActor *);
extern void holdRaceSetupNamePlate(MenuScreenEffectActor *);
extern void drawCongratulationsBannerModel(MenuScreenEffectActor *);
extern void updateCongratulationsBannerModel(MenuScreenEffectActor *);
extern void updatePassAwardLabelFadeIn(MenuScreenEffectActor *);
extern void drawMainMenuRotatingBoardModel(MenuScreenEffectActor *);
extern void drawMainMenuStaticBoardModel(void *);
extern void renderMainMenuBoardModels(MenuScreenEffectActor *arg0);

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/drawFallingConfetti.s")

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

void stopPassAwardWinner(MenuScreenEffectActor *arg0) {
    loopMainMenuSceneModelAnimation(0);
    addMainMenuSceneModelDrawCallbackForViewport0(0);
}

void updatePassAwardWinner(MenuScreenEffectActor *arg0) {
    if (gMenuFadeAlpha == 0) {
        if (arg0->unk18.byte.b0 != 0) {
            enqueueSoundEffect(raceSetupCharacterFocusSoundIds[arg0->index], 0x32);
            arg0->unk18.byte.b0 = 0;
        }
        loopMainMenuSceneModelAnimation(0);
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)stopPassAwardWinner);
        addMainMenuSceneModelDrawCallbackForViewport0(0);
        return;
    }
    addMainMenuSceneModelDrawCallback(0);
}

void initPassAwardWinner(MenuScreenEffectActor *arg0) {
    arg0->unk18.byte.b0 = 1;
    initMainMenuSceneModel(0, arg0->index);
    setMainMenuSceneModelAnimation(0, raceSetupCharacterFocusAnimationIds[arg0->index]);
    loopMainMenuSceneModelAnimation(0);
    setMainMenuSceneModelPosition(0, 0, 0, 0);
    setMainMenuSceneModelRotation(0, 0, 0, 0);
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updatePassAwardWinner);
}

void updatePassAwardOpponent(MenuScreenEffectActor *arg0) {
    s16 temp = arg0->unk1C.half.hi;

    if (temp == 0) {
        loopMainMenuSceneModelAnimation(arg0->unk18.word);
    } else {
        arg0->unk1C.half.hi = temp - 1;
    }
    addMainMenuSceneModelDrawCallbackForViewport(arg0->unk18.word, 1);
}

void initPassAwardOpponent(MenuScreenEffectActor *arg0) {
    s32 a = arg0->unk18.word;
    arg0->unk1C.half.hi = a * 0xF;
    initMainMenuSceneModel(a, arg0->index);
    setMainMenuSceneModelAnimation(arg0->unk18.word, passAwardOpponentAnimationIds[arg0->index]);
    loopMainMenuSceneModelAnimation(arg0->unk18.word);
    {
        s32 b = arg0->unk18.word;
        setMainMenuSceneModelPosition(b, passAwardOpponentXPositions[b], 0xFFB50000, 0xA00000);
    }
    setMainMenuSceneModelRotation(arg0->unk18.word, 0, 0x800, 0);
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updatePassAwardOpponent);
}

void createPassAwardOpponent(s32 arg0, s32 arg1) {
    MenuScreenEffectActor *p = createCallbackTaskWithUserId((CallbackTaskCallback)initPassAwardOpponent, 0, 0x64, arg1);
    if (p != NULL) {
        p->unk18.word = arg0;
    }
}

void drawTitleScreenStartPrompt(MenuScreenEffectActor *arg0) {
    drawMenuAsciiTextDefaultScale(-0x44, 0x38, D_800E0520, 0);
}

void updateTitleScreenStartPrompt(void *arg0) {
    if (gFrameCounter & 8) {
        addRenderCallback(&gMenuForegroundRenderCallbackList, (RenderCallback)drawTitleScreenStartPrompt, 0);
    }
}

void drawRaceStartPlayerEffectSprite(MenuScreenEffectActor *arg0) {
    void *spA4;
    void *spA0;
    volatile u8 gap[0xC];
    Transform3D sp74;

    if (gRenderMatricesDirty != 0) {
        arg0->unk2F = 1;
    }

    if (arg0->unk2F != 0) {
        arg0->unk2F = 0;
        sp74 = gIdentityFixedTransform;
        sp74.translation.x = arg0->unk18.word;
        sp74.translation.y = arg0->unk1C.word;
        sp74.translation.z = arg0->unk20.word;
        arg0->unk24.word = (s32)allocFixedTransformMatrix(&sp74);
    }

    if (arg0->unk24.word != 0) {
        gSPDisplayList(gRegionAllocPtr++, gAlphaSpriteRenderModeDl);
        getAssetTableImageAndPalette(
            getRelocatableHeapBlockBase(gAssetHandles[30]),
            (u16)(arg0->unk2E + 0x24),
            &spA4,
            &spA0
        );
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xFD500000, (u32)spA4);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xF5500000, 0x07080200);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xE6000000, 0);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xF3000000, 0x070FF400);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xE7000000, 0);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xF5400400, 0x80200);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xF2000000, 0x7C07C);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xFD100000, (u32)spA0);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xE8000000, 0);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xF5000100, 0x07000000);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xE6000000, 0);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xF0000000, 0x0703C000);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xE7000000, 0);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0x01020040, arg0->unk24.word);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0x01000040, (u32)gViewportMatrix);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0x0400103F, (u32)raceStartPlayerEffectVertices);
        MAIN_MENU_GFX_CMD(gRegionAllocPtr++, 0xB1060402, 0x60200);
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/func_80054004.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/func_80054174.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/drawTitleMenuSparkle.s")

void renderTitleMenuSparkle(MenuScreenEffectActor *arg0) {
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawTitleMenuSparkle, arg0);
}

void initTitleMenuSparkle(MenuScreenEffectActor *arg0) {
    s16 *temp = &titleMenuSparklePositions[arg0->index * 2];

    arg0->unk18.half.hi = temp[0];
    arg0->unk18.half.lo = temp[1];
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)renderTitleMenuSparkle);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/drawCourseRecordBanner.s")

void updateCourseRecordBannerFadeOut(MenuScreenEffectActor *arg0) {
    s16 v = arg0->unk18.half.hi;
    if (v != 0) {
        arg0->unk18.half.hi = v - 1;
    } else {
        arg0->unk18.half.lo -= 0x20;
        if (arg0->unk18.half.lo <= 0) {
            removeCallbackTask(arg0);
            return;
        }
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCourseRecordBanner, arg0);
}

void initCourseRecordBannerFadeOut(MenuScreenEffectActor *arg0) {
    arg0->unk18.half.hi = 0x3E;
    arg0->unk18.half.lo = 0xFF;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseRecordBannerFadeOut);
}

void drawRaceSetupNamePlate(MenuScreenEffectActor *arg0) {
    u8 *new_var;

    new_var = &gCurrentViewportIndex;
    if (arg0->index == *new_var) {
        drawAssetTableSprite(
            (s16)((arg0->unk18.word >> 1) - 0x38),
            -8,
            getRelocatableHeapBlockBase(gAssetHandles[31]),
            0x76
        );
    }
}

void updateRaceSetupNamePlateSlideOut(MenuScreenEffectActor *arg0) {
    s32 temp_v0 = arg0->unk24.word;
    s32 temp_t8 = (unsigned long long)(temp_v0 + 4);

    arg0->unk18.word += temp_v0;
    arg0->unk24.word = temp_t8;
    if (temp_t8 == 0x38) {
        removeCallbackTask(arg0);
    } else {
        addRenderCallback(&gRaceOverlayRenderCallbackList, (RenderCallback)drawRaceSetupNamePlate, (void *)arg0);
    }
}

void holdRaceSetupNamePlate(MenuScreenEffectActor *arg0) {
    u16 temp = (arg0->timer & 0xFFFFU) - 1;

    arg0->timer = temp;
    temp += 0;
    if (temp == 0) {
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateRaceSetupNamePlateSlideOut);
    }
    if (1) {}
    addRenderCallback(&gRaceOverlayRenderCallbackList, (RenderCallback)drawRaceSetupNamePlate, (void *)arg0);
}

void updateRaceSetupNamePlateSlideIn(MenuScreenEffectActor *arg0) {
    arg0->unk18.word += arg0->unk24.word;
    arg0->unk24.word -= 4;
    if (arg0->unk24.word == 0) {
        arg0->timer = 0x1E;
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)holdRaceSetupNamePlate);
    }
    addRenderCallback(&gRaceOverlayRenderCallbackList, (RenderCallback)drawRaceSetupNamePlate, (void *)arg0);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/func_800546D0.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/drawCongratulationsBannerModel.s")

void updateCongratulationsBannerAfterimage(MenuScreenEffectActor *arg0) {
    arg0->alpha -= 0x20;
    if (arg0->alpha == 0) {
        removeCallbackTask(arg0);
        return;
    }
    addRenderCallback(&gEffectRenderCallbackList, (RenderCallback)drawCongratulationsBannerModel, (void *)arg0);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/updateCongratulationsBannerModel.s")

void initCongratulationsBannerModel(MenuScreenEffectActor *arg0) {
    arg0->alpha = 0xFF;
    arg0->unk30.word = 0;
    arg0->unk18.word = 0;
    arg0->unk1C.word = 0x300000;
    arg0->unk20.word = 0xEC0000;
    arg0->spriteIndex = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCongratulationsBannerModel);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/func_80054C18.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/func_80054C84.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/renderer/menu_screen_effects/drawPassAwardLabel.s")

void updatePassAwardLabelFadeIn(MenuScreenEffectActor *arg0) {
    s16 v = arg0->unk18.half.hi;
    if (v != 0) {
        arg0->unk18.half.hi = v - 1;
        return;
    }
    arg0->unk18.half.lo += 0x20;
    if (arg0->unk18.half.lo >= 0x100) {
        arg0->unk18.half.lo = 0xFF;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawPassAwardLabel, arg0);
}

void initPassAwardLabelFadeIn(MenuScreenEffectActor *arg0) {
    arg0->unk18.half.hi = 0x50;
    arg0->unk18.half.lo = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updatePassAwardLabelFadeIn);
}

void drawMainMenuRotatingBoardModel(MenuScreenEffectActor *arg0) {
    TitleMenuRotatingBoardScratch scratch;
    Mtx *matrix;

    if (gCurrentViewportIndex == 0) {
        makeFixedRotationY(scratch.source.rotation, arg0->unk18.half.hi);
        scratch.source.translation.x = 0;
        scratch.source.translation.y = 0;
        scratch.source.translation.z = 0;
        matrix = allocFixedTransformMatrix(&scratch.source);
        if (matrix != NULL) {
            gDPPipeSync(gRegionAllocPtr++);

            gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(gAssetHandles[8]));
            gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(gAssetHandles[9]));

            gSPMatrix(gRegionAllocPtr++, matrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

            gSPDisplayList(gRegionAllocPtr++, D_2000E48);
        }
    }
}

void drawMainMenuStaticBoardModel(void *arg0) {
    if (gCurrentViewportIndex == 0) {
        gDPPipeSync(gRegionAllocPtr++);

        gSPSegment(gRegionAllocPtr++, 0x02, getRelocatableHeapBlockBase(gAssetHandles[8]));
        gSPSegment(gRegionAllocPtr++, 0x03, getRelocatableHeapBlockBase(gAssetHandles[9]));

        gSPMatrix(gRegionAllocPtr++, &gIdentityMatrix, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        gSPDisplayList(gRegionAllocPtr++, D_2000E70);
    }
}

void renderMainMenuBoardModels(MenuScreenEffectActor *arg0) {
    addRenderCallback(&gBackdropRenderCallbackList, (RenderCallback)drawMainMenuStaticBoardModel, arg0);
    addRenderCallback(&gRaceObjectRenderCallbackList, (RenderCallback)drawMainMenuRotatingBoardModel, arg0);
}

void initMainMenuBoardModels(MenuScreenEffectActor *arg0) {
    arg0->unk18.half.hi = 0x360;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)renderMainMenuBoardModels);
}
