#include "common.h"
#include "game/menu/menu_scratch.h"
#include "game/race/race_state.h"
#include "font_encoding.h"
#include <PR/os_libc.h>
#include "game/save_data.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/game_task_scheduler.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/character_select/character_select_menu.h"
#include "game/menu/course_select/course_select_menu.h"
#include "game/menu/course_select/course_select_ui.h"
#include "game/menu/course_select/course_select_shop_ui.h"
#include "game/menu/main_menu/controller_main_menu_flow.h"
#include "game/math/fixed_point_math.h"
#include "game/race/player/race_player_model_renderer.h"
#include "game/race/player/race_player_input.h"

extern u8 gCurrentViewportIndex;

extern u8 D_800EC9C0;

extern s16 gCourseSelectIconListXLayout[][4];
extern s16 gCourseSelectIconListYLayout[][6];
extern s16 gCourseSelectStatsPanelLayout[][4];
extern const char D_800E0DB8[];

void drawCourseSelectSnowboardPreviewIn(CourseSelectCoursePreviewActor *arg0) {
    u8 sp2F;
    unsigned char sp2E;
    u8 var_t0;
    s8 temp_v0_2;
    RacePlayer *temp_v0_3;
    Transform3D sp30;
    u8 var_a3;
    u8 var_v1;
    int temp_v0;

    if ((gCourseSelectViewportSyncState != 0) && (gCurrentViewportIndex == 1)) {
        var_t0 = 0;
    } else {
        var_t0 = gCurrentViewportIndex;
    }
    if ((gCourseSelectSlideStates[var_t0] == 0) || (gCourseSelectSlideStates[var_t0] & 1)) {
        temp_v0 = arg0->playerFlags[var_t0];
        if ((temp_v0 == 0) || (temp_v0 & 1)) {
            if (gCourseSelectSlideStates[var_t0] == 1) {
                var_a3 = arg0->playerSlots[var_t0].courseIndex;
            } else {
                var_a3 = gRacePlayers[var_t0].menuSelection;
            }
            temp_v0_2 = gGameSaveDataBuffer[var_t0].courseUnlockStates[var_a3];
            if (temp_v0_2 == -1) {
                var_v1 = 9;
            } else {
                var_v1 = temp_v0_2;
            }
            if ((gCourseSelectViewportSyncState != 0) && (gCurrentViewportIndex == 1)) {
                var_v1 = (u8)(gCourseSelectViewportSyncState - 1);
            }
            temp_v0_3 = &gRacePlayers[gCurrentViewportIndex];
            if (temp_v0_3->selectedCharacterId == 5) {
                var_v1 = 0;
                var_a3 = (var_a3 % 3) + 0xC;
            }
            if (temp_v0_3->menuSelection >= 9) {
                var_v1 = 0;
            }
            sp2E = var_v1;
            sp2F = var_a3;
            composeFixedTransforms(&arg0->viewTransform, &arg0->modelTransforms[var_t0], &sp30);
            arg0->renderMatrix = allocFixedTransformMatrix(&sp30);
            if (arg0->renderMatrix != 0) {
                drawSnowboardModel(arg0->renderMatrix, (s16)sp2F, (s16)sp2E);
            }
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/updateCourseSelectPreviewModelIn.s")

void initCourseSelectPreviewModelIn(void *arg0) {
    CourseSelectAnimatedActor *actor;
    Vec3i position;
    s32 i;

    actor = arg0;
    i = 0;
    do {
        actor->modelOffsets[i].x = 0xC00000;
        actor->modelOffsets[i].y = 0;
        actor->modelOffsets[i].z = 0;
        actor->rotationAngle[i] = 0;
        makeFixedRotationY(actor->modelTransforms[i].rotation, actor->rotationAngle[i]);
        transformVec3iByFixedMatrix(actor->modelTransforms[i].rotation, &actor->modelOffsets[i], &position);
        actor->modelTransforms[i].translation.x = position.x;
        actor->modelTransforms[i].translation.y = position.y;
        actor->modelTransforms[i].translation.z = position.z;
        i++;
        actor->transitionState[i - 1] = 0;
    } while (i < 4);

    actor->viewTransform.translation.x = 0;
    actor->viewTransform.translation.y = 0;
    actor->viewTransform.translation.z = 0;
    makeFixedRotationYX(actor->viewTransform.rotation, 0x400, 0x280);
    setCallbackTaskCallback(actor, (CallbackTaskCallback)updateCourseSelectPreviewModelIn);
}

void drawCourseSelectSnowboardPreviewOut(CourseSelectCoursePreviewActor *arg0) {
    u8 sp2F;
    unsigned char sp2E;
    u8 var_t0;
    s8 temp_v0_2;
    RacePlayer *temp_v0_3;
    Transform3D sp30;
    u8 var_a3;
    u8 var_v1;
    int temp_v0;

    if ((gCourseSelectViewportSyncState != 0) && (gCurrentViewportIndex == 1)) {
        var_t0 = 0;
    } else {
        var_t0 = gCurrentViewportIndex;
    }
    if ((gCourseSelectSlideStates[var_t0] == 2) || (gCourseSelectSlideStates[var_t0] & 1)) {
        temp_v0 = arg0->playerFlags[var_t0];
        if ((temp_v0 == 0) || (temp_v0 & 1)) {
            if (gCourseSelectSlideStates[var_t0] == 3) {
                var_a3 = arg0->playerSlots[var_t0].courseIndex;
            } else {
                var_a3 = gRacePlayers[var_t0].menuSelection;
            }
            temp_v0_2 = gGameSaveDataBuffer[var_t0].courseUnlockStates[var_a3];
            if (temp_v0_2 == -1) {
                var_v1 = 9;
            } else {
                var_v1 = temp_v0_2;
            }
            if ((gCourseSelectViewportSyncState != 0) && (gCurrentViewportIndex == 1)) {
                var_v1 = (u8)(gCourseSelectViewportSyncState - 1);
            }
            temp_v0_3 = &gRacePlayers[gCurrentViewportIndex];
            if (temp_v0_3->selectedCharacterId == 5) {
                var_v1 = 0;
                var_a3 = (var_a3 % 3) + 0xC;
            }
            if (temp_v0_3->menuSelection >= 9) {
                var_v1 = 0;
            }
            sp2E = var_v1;
            sp2F = var_a3;
            composeFixedTransforms(&arg0->viewTransform, &arg0->modelTransforms[var_t0], &sp30);
            arg0->renderMatrix = allocFixedTransformMatrix(&sp30);
            if (arg0->renderMatrix != 0) {
                drawSnowboardModel(arg0->renderMatrix, (s16)sp2F, (s16)sp2E);
            }
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/updateCourseSelectPreviewModelOut.s")

void initCourseSelectPreviewModelOut(void *arg0) {
    CourseSelectAnimatedActor *actor;
    Vec3i position;
    s32 i;

    actor = arg0;
    for (i = 0; i < 4; i++) {
        actor->modelOffsets[i].x = 0;
        actor->modelOffsets[i].y = 0x800000;
        actor->modelOffsets[i].z = 0;
        actor->rotationAngle[i] = 0;
        makeFixedRotationY(actor->modelTransforms[i].rotation, actor->rotationAngle[i]);
        transformVec3iByFixedMatrix(actor->modelTransforms[i].rotation, &actor->modelOffsets[i], &position);
        actor->modelTransforms[i].translation.x = position.x;
        actor->modelTransforms[i].translation.y = position.y;
        actor->modelTransforms[i].translation.z = position.z;
        actor->transitionState[i] = 1;
        gCourseSelectStatus.outgoingPreviewModelState[i] = 1;
    }

    actor->viewTransform.translation.x = 0;
    actor->viewTransform.translation.y = 0;
    actor->viewTransform.translation.z = 0;
    makeFixedRotationYX(actor->viewTransform.rotation, 0x400, 0x280);
    setCallbackTaskCallback(actor, (CallbackTaskCallback)updateCourseSelectPreviewModelOut);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/func_80024A20.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/func_80024E28.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/initCourseSelectCourseIconList.s")

void drawCourseSelectCourseCursors(CourseSelectCourseCursorsActor *arg0) {
    u8 *actor;
    s16 *drawPosPtr;
    s32 i;
    u8 *statePtr;
    s16 *posPtr;
    int cursorStride;
    s16 *handles;
    u8 *cursorBytes;
    s32 tileIndex;
    s32 handleIndex;
    s32 one;
    s32 playerCount;
    u8 *cursorBase;

    actor = (u8 *)arg0;
    if (gCourseSelectInputLocked == 0) {
        playerCount = gPlayerCount;
        i = 0;
        if ((playerCount - 1) >= 0) {
            statePtr = (u8 *)arg0;
            cursorStride = 2;
            handles = gAssetHandles;
            one = 1;
            do {
                if (statePtr[0x30] < cursorStride) {
                    cursorBase = actor;
                    posPtr = (s16 *)((cursorBytes = cursorBase) + (i * cursorStride));
                    if (one == gPlayerCount) {
                        tileIndex = 7;
                        handleIndex = 0x25;
                    } else {
                        tileIndex = 0x13;
                        handleIndex = 0x21;
                    }
                    drawMenuSpriteWithAlpha(
                        posPtr[0xC],
                        (s16)((drawPosPtr = posPtr)[0x10] +
                              (gCharacterSelectHighlightedRosterIndices[i] * statePtr[0x38])),
                        getRelocatableHeapBlockBase(handles[handleIndex]),
                        tileIndex,
                        0x20,
                        0x20,
                        0,
                        drawPosPtr[0x14],
                        (short)0
                    );
                }
                i++;
                statePtr++;
            } while (i < gPlayerCount);
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/updateCourseSelectCourseCursors.s")

void initCourseSelectCourseCursors(CourseSelectCourseCursorsActor *arg0) {
    CourseSelectCourseCursorsActor *actor;
    u8 *courseUnlocked;
    s16 *yLayout;
    s16 *xLayout;
    char mask;
    s32 unlockState;
    s32 hasExtraCourse;
    s32 playerIndex;
    s32 layoutIndex;

    actor = arg0;
    // clang-format off
    mask = 0xFFFFFFFFFFFFFFFFu; \
    if ((s32)gPlayerCount < 3) { \
        layoutIndex = gPlayerCount - 1; \
    } else { \
        layoutIndex = 2; \
    } \
    playerIndex = 0; \
    if ((s32)gPlayerCount > 0) { \
        courseUnlocked = gCourseSelectHasExtraCourse; \
        yLayout = gCourseSelectIconListYLayout[layoutIndex]; \
        xLayout = gCourseSelectIconListXLayout[layoutIndex]; \
        do { \
            unlockState = *courseUnlocked & mask; \
            courseUnlocked++; \
            hasExtraCourse = (((0, unlockState) == 0) || (gRacePlayers[playerIndex].selectedCharacterId == 5)) ? 0 : 1;
            // clang-format on
            actor->horizontalSpacing[playerIndex] = yLayout[hasExtraCourse];
            actor->y[playerIndex] = yLayout[((playerIndex & 1) * 2) + hasExtraCourse + 2];
            actor->x[playerIndex] = xLayout[((playerIndex >= 2) * 2) + 1];
            actor->alpha[playerIndex] = 0;
            actor->state[playerIndex] = 0;
            actor->pulseTimer[playerIndex] = 0;
            playerIndex++;
        } while (playerIndex < (s32)gPlayerCount);
    }
    setCallbackTaskCallback(actor, (CallbackTaskCallback)updateCourseSelectCourseCursors);
}

void drawCourseSelectCourseListBackdrop(CourseSelectCourseListBackdropActor *arg0) {
    drawMenuSprite(arg0->x, arg0->y, getRelocatableHeapBlockBase(gAssetHandles[0x25]), 3, 0x20, 0x20, 0, 0);
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        arg0->y,
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        4,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        arg0->x,
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        5,
        0x20,
        0x20,
        0,
        0
    );
    drawMenuSprite(
        (s16)(arg0->x + 0x40),
        (s16)(arg0->y + 0x40),
        getRelocatableHeapBlockBase(gAssetHandles[0x25]),
        6,
        0x20,
        0x20,
        0,
        0
    );
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/updateCourseSelectCourseListBackdrop.s")

void initCourseSelectCourseListBackdrop(CourseSelectCourseListBackdropActor *arg0) {
    arg0->x = -0x108;
    arg0->y = 8;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseSelectCourseListBackdrop);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/drawCourseSelectCourseStats.s")

void updateCourseSelectCourseStats(CourseSelectCourseStatsActor *arg0) {
    s32 state = arg0->state;

    switch (state ^ 0) {
        case 0:
            arg0->alpha += 0x26;
            if (arg0->alpha >= 0x100) {
                arg0->alpha = 0x100;
                arg0->state = 1;
            }
            state = arg0->state;
            break;
        case 1:
            if (gRacePlayers[0].menuState == 3) {
                arg0->state = 2;
            }
            if (gCurrentGameTask->callbackData2 == 9) {
                arg0->state = 4;
            }
            state = arg0->state;
            break;
        case 2:
            arg0->alpha -= 0x40;
            if (arg0->alpha <= 0) {
                arg0->alpha = 0;
                arg0->state = 3;
            }
            state = arg0->state;
            break;
        case 3:
            state = (arg0->state = 4);
            break;
    }

    if (state == 4) {
        removeCallbackTask(arg0);
        finishCourseSelectUiTask(5);
        return;
    }

    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCourseSelectCourseStats, arg0);
}

void initCourseSelectCourseStats(CourseSelectCourseStatsActor *arg0) {
    s32 row;
    s32 i;
    CourseSelectCourseStatsActor *temp_a3;
    s16(*table)[4];

    temp_a3 = arg0;
    if (gPlayerCount < 3) {
        row = gPlayerCount - 1;
    } else {
        row = 2;
    }

    i = 0;
    if ((s32)gPlayerCount > 0) {
        table = &gCourseSelectStatsPanelLayout[row];
        do {
            temp_a3->x[i] = (*table)[(i & 1) * 2] + ((i >= 2) * 0x8C);
            temp_a3->y[i] = (*table)[(i & 1) * 2 + 1];
            i++;
        } while (i < (s32)gPlayerCount);
    }

    temp_a3->alpha = 0;
    temp_a3->state = 0;
    setCallbackTaskCallback(temp_a3, (CallbackTaskCallback)updateCourseSelectCourseStats);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/func_80026C78.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/updateCourseSelectCourseDescription.s")

void initCourseSelectCourseDescription(CourseSelectCourseDescriptionActor *arg0) {
    arg0->x = -0x84;
    arg0->y = 0xC;
    arg0->alpha = 0;
    arg0->state = 0;
    arg0->selectionLocked = 0;
    arg0->selectedBoardLevel = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseSelectCourseDescription);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/drawCourseSelectExtraCourseBadge.s")

void updateCourseSelectExtraCourseBadge(CourseSelectExtraCourseBadgeActor *arg0) {
    u8 state = arg0->state;

    switch (state) {
        case 0:
            arg0->alpha += 0x26;
            if (arg0->alpha >= 0x100) {
                arg0->alpha = 0x100;
                arg0->state = 1;
            }
            state = arg0->state;
            break;
        case 1:
            if ((gRacePlayers[0].menuState == 3) || (gCurrentGameTask->callbackData2 == 9)) {
                state = (arg0->state = 2);
            }
            break;
        case 2:
            arg0->x += 0x20;
            break;
    }
    state = arg0->state;
    if ((state == 2) && (arg0->x >= 0x94)) {
        removeCallbackTask(arg0);
        finishCourseSelectUiTask(7);
        return;
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCourseSelectExtraCourseBadge, arg0);
}

void initCourseSelectExtraCourseBadge(CourseSelectExtraCourseBadgeActor *arg0) {
    arg0->x = -0x8;
    arg0->y = -0x5C;
    arg0->alpha = 0;
    arg0->state = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseSelectExtraCourseBadge);
}

void drawCourseSelectExtraCourseIconList(CourseSelectExtraCourseIconListActor *arg0) {
    s32 overlayTile;
    s32 iconAlpha;
    s32 playerIndex;
    s32 iconIndex;

    for (playerIndex = 0; playerIndex < gPlayerCount; playerIndex++) {
        if (gMenuChoicePromptState[playerIndex] != 0) {
            if (gMenuChoicePromptState[playerIndex] != 9) {
                overlayTile = (playerIndex >= 2) * 0x8C;
                for (iconIndex = arg0->itemCounts[playerIndex] - 1; iconIndex >= 0; iconIndex--) {
                    if (gMenuChoicePromptState[playerIndex] == 1) {
                        iconAlpha = 0x100;
                    } else if ((iconIndex + 2) == gMenuChoicePromptState[playerIndex]) {
                        iconAlpha = 0x100;
                    } else {
                        iconAlpha = 0x60;
                    }
                    drawMenuSpriteWithAlphaClipped(
                        arg0->iconX[playerIndex][iconIndex],
                        arg0->iconY[playerIndex][iconIndex],
                        getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                        arg0->tileIndices[playerIndex][iconIndex],
                        0x20,
                        0x20,
                        0,
                        iconAlpha,
                        0,
                        arg0->clipLeft - overlayTile,
                        arg0->clipTop,
                        arg0->clipRight,
                        arg0->clipBottom
                    );
                }
                if (gCourseSelectInputLocked == 0) {
                    if ((gMenuChoicePromptState[playerIndex] >= 2) && (gMenuChoicePromptState[playerIndex] < 5)) {
                        if (gPlayerCount == 1) {
                            overlayTile = 0xF;
                        } else {
                            overlayTile = 0x1B;
                        }
                        drawMenuSpriteWithAlpha(
                            arg0->iconX[playerIndex][0],
                            (s16)(arg0->iconY[playerIndex][0] +
                                  ((gMenuChoicePromptState[playerIndex] - 2) * arg0->rowSpacing[playerIndex])),
                            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                            overlayTile,
                            0x20,
                            0x20,
                            0,
                            ((CourseSelectExtraCourseIconListActor *)&((u8 *)&arg0->task)[playerIndex * 2])->alpha[0],
                            0
                        );
                    }
                }
            }
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/func_80027868.s")

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/updateCourseSelectExtraCourseIconListOut.s")

void updateCourseSelectExtraCourseIconListClose(CourseSelectExtraCourseIconListActor *arg0) {
    s32 i;
    s32 j;
    CourseSelectExtraCourseIconListActor *temp_a2 = arg0;

    i = 0;
    if ((s32)gPlayerCount > 0) {
        do {
            j = 0;
            if ((s32)arg0->itemCounts[i] > 0) {
                do {
                    temp_a2->iconX[i][j] -= 0x20;
                    j++;
                } while (j < (s32)arg0->itemCounts[i]);
            }
            i++;
        } while (i < (s32)gPlayerCount);
    }

    if ((gPlayerCount == 1) && (temp_a2->itemCounts[0] == 0)) {
        removeCallbackTask(temp_a2);
        return;
    }

    if (temp_a2->iconX[0][0] < -0xDF) {
        removeCallbackTask(temp_a2);
        finishCourseSelectUiTask(8);
        gMenuScratch2.task = 0;
        return;
    }

    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCourseSelectExtraCourseIconList, temp_a2);
}

void updateCourseSelectExtraCourseIconList(CourseSelectExtraCourseIconListActor *arg0) {
    CourseSelectExtraCourseIconListActor *actor;
    s16 *promptState;
    s32 playerIndex;
    s32 iconIndex;
    s32 layoutIndex;
    s32 iconMask;
    s32 newCount;
    s16 direction;
    s16 state;
    u8 splitscreenMode;
    u8 count;

    actor = (CourseSelectExtraCourseIconListActor *)arg0;
    if (gRacePlayers[0].menuState == 3) {
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseSelectExtraCourseIconListClose);
        for (playerIndex = 0; playerIndex < gPlayerCount; playerIndex++) {
            actor->alpha[playerIndex] = 0x100;
        }
    } else if (gCurrentGameTask->callbackData2 == 1) {
        setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseSelectExtraCourseIconListOut);
        actor->alpha[0] = 0x100;
        actor->pulseTimer[0] = 0;
    } else {
        for (playerIndex = 0; playerIndex < gPlayerCount; playerIndex++) {
            if (gRacePlayers[playerIndex].menuState != 2) {
                promptState = &gMenuChoicePromptState[playerIndex];
                if ((*promptState == 1) || ((*promptState >= 5) && (*promptState < 9))) {
                    if ((*promptState == 1) && (actor->itemCounts[playerIndex] == 0)) {
                        iconMask = 1;
                        if (gRacePlayers[playerIndex].selectedCharacterId == 5) {
                            actor->itemCounts[playerIndex] = 1;
                            if (gPlayerCount == 1) {
                                actor->tileIndices[playerIndex][0] = 9;
                            } else {
                                actor->tileIndices[playerIndex][0] = 0x1C;
                            }
                        } else {
                            for (iconIndex = 0; iconIndex < 3; iconIndex++) {
                                if (gRacePlayers[playerIndex].menuSelection >= 9) {
                                    if (gRaceSplitscreenMode == 3) {
                                        if (gGameSaveDataBuffer[playerIndex].extraCourseUnlockFlags & iconMask) {
                                            actor->itemCounts[playerIndex]++;
                                            if (gPlayerCount == 1) {
                                                actor->tileIndices[playerIndex][actor->itemCounts[playerIndex] - 1] =
                                                    iconIndex + 0xC;
                                            } else {
                                                actor->tileIndices[playerIndex][actor->itemCounts[playerIndex] - 1] =
                                                    iconIndex + 0x1F;
                                            }
                                        }
                                        iconMask *= 2;
                                    } else if (gGameSaveDataBuffer[playerIndex].courseUnlockStates[iconIndex + 9] !=
                                               -1) {
                                        actor->itemCounts[playerIndex]++;
                                        if (gPlayerCount == 1) {
                                            actor->tileIndices[playerIndex][actor->itemCounts[playerIndex] - 1] =
                                                iconIndex + 0xC;
                                        } else {
                                            actor->tileIndices[playerIndex][actor->itemCounts[playerIndex] - 1] =
                                                iconIndex + 0x1F;
                                        }
                                    }
                                } else if (gGameSaveDataBuffer[playerIndex].courseUnlockStates
                                               [(gRacePlayers[playerIndex].menuSelection % 3) + (iconIndex * 3)] !=
                                           -1) {
                                    actor->itemCounts[playerIndex]++;
                                    if (gPlayerCount == 1) {
                                        actor->tileIndices[playerIndex][iconIndex] = iconIndex + 9;
                                    } else {
                                        actor->tileIndices[playerIndex][iconIndex] = iconIndex + 0x1C;
                                    }
                                }
                            }

                            splitscreenMode = gRaceSplitscreenMode;
                            if ((gRaceSplitscreenMode == 3) && (gCourseSelectModeSelection == 0)) {
                                if (gRacePlayers[playerIndex].menuSelection < 9) {
                                    actor->itemCounts[playerIndex]++;
                                }
                                if (actor->itemCounts[playerIndex] >= 4) {
                                    actor->itemCounts[playerIndex] = 3;
                                }
                                if (gRacePlayers[playerIndex].menuSelection < 9) {
                                    actor->tileIndices[playerIndex][actor->itemCounts[playerIndex] - 1] =
                                        actor->itemCounts[playerIndex] + 8;
                                }
                            }
                        }
                    }

                    if (*promptState == 1) {
                        direction = 1;
                    } else {
                        direction = -1;
                    }
                    for (iconIndex = 0; iconIndex < (s32)actor->itemCounts[playerIndex]; iconIndex++) {
                        if (direction == 1) {
                            if ((s32)actor->revealTimer[playerIndex] < (iconIndex * 3) + 3) {
                                actor->iconY[playerIndex][iconIndex] += (s32)actor->rowSpacing[playerIndex] / 3;
                            }
                        } else if ((iconIndex * 2) + 2 >= (s32)actor->revealTimer[playerIndex]) {
                            actor->iconY[playerIndex][iconIndex] -= (s32)actor->rowSpacing[playerIndex] / 2;
                        }
                    }

                    if (direction == 1) {
                        actor->revealTimer[playerIndex]++;
                    } else {
                        actor->revealTimer[playerIndex]--;
                    }
                    if (direction == 1) {
                        if (actor->revealTimer[playerIndex] == (actor->itemCounts[playerIndex] * 3)) {
                            *promptState = gCourseSelectSelectedRows[playerIndex] + 2;
                        }
                    } else if (actor->revealTimer[playerIndex] == 0) {
                        *promptState = 0;
                        actor->itemCounts[playerIndex] = 0;
                        if ((s32)gPlayerCount < 3) {
                            layoutIndex = gPlayerCount - 1;
                        } else {
                            layoutIndex = 2;
                        }
                        for (iconIndex = 0; iconIndex < 3; iconIndex++) {
                            actor->iconY[playerIndex][iconIndex] =
                                gCourseSelectIconListYLayout[layoutIndex][((playerIndex & 1) * 2) + 2];
                        }
                    }
                }

                state = *promptState;
                if ((state >= 2) && (state < 5)) {
                    if ((s32)actor->pulseTimer[playerIndex] < 0x10) {
                        actor->alpha[playerIndex] -= 9;
                    } else {
                        actor->alpha[playerIndex] += 9;
                    }
                    actor->pulseTimer[playerIndex] = (actor->pulseTimer[playerIndex] + 1) & 0x1F;
                }
            }

            gCourseSelectStatus.extraCourseCounts[playerIndex] = actor->itemCounts[playerIndex];
        }
    }
    addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCourseSelectExtraCourseIconList, arg0);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/initCourseSelectExtraCourseIconList.s")

void drawCourseSelectPlayerPanels(CourseSelectPlayerPanelListActor *arg0) {
    s32 j;
    s32 i;
    s32 middleCount;
    u8 tile;
    s32 alpha;
    s32 count;
    u8 text[4];
    s32 offset;
    CourseSelectPlayerPanelListActor *actor = (CourseSelectPlayerPanelListActor *)arg0;

    if (gPlayerCount == 2) {
        count = 2;
    } else {
        count = 4;
    }

    for (i = 0; i < count; i++) {
        if (i == gPlayerCount) {
            alpha = 0xC0;
        } else {
            alpha = 0x100;
        }

        if (i == 0) {
            tile = 2;
        } else {
            tile = i + 5;
        }

        drawMenuSpriteWithAlphaWideArgs(
            actor->x[i],
            actor->y[i],
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x22,
            0x20,
            0x20,
            0,
            alpha,
            tile
        );

        if (gPlayerCount == 2) {
            middleCount = 0xB;
        } else {
            middleCount = 3;
        }

        for (j = 0, offset = 0x38; j < middleCount; j++, offset += 0x10) {
            drawMenuSpriteWithAlphaWideArgs(
                (s16)(actor->x[i] + offset),
                actor->y[i],
                getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                0x23,
                0x20,
                0x20,
                0,
                alpha,
                tile
            );
        }

        drawMenuSpriteWithAlphaWideArgs(
            (s16)(actor->x[i] + offset),
            actor->y[i],
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x24,
            0x20,
            0x20,
            0,
            alpha,
            tile
        );

        if (gPlayerCount == 2) {
            offset = 0xF8;
        } else {
            offset = 0x78;
        }

        for (j = 0; j < 4; j++) {
            drawMenuSpriteWithAlphaWideArgs(
                actor->x[i],
                (s16)(actor->y[i] + j * 0x10 + 0x10),
                getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                0x25,
                0x20,
                0x20,
                0,
                alpha,
                tile
            );

            drawMenuSpriteWithAlphaWideArgs(
                (s16)(actor->x[i] + offset),
                (s16)(actor->y[i] + j * 0x10 + 0x10),
                getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                0x26,
                0x20,
                0x20,
                0,
                alpha,
                tile
            );
        }

        drawMenuSpriteWithAlphaWideArgs(
            actor->x[i],
            (s16)(actor->y[i] + 0x50),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x27,
            0x20,
            0x20,
            0,
            alpha,
            tile
        );

        if (gPlayerCount == 2) {
            middleCount = 0xE;
        } else {
            middleCount = 6;
        }

        for (j = 0; j < middleCount; j++) {
            drawMenuSpriteWithAlphaWideArgs(
                (s16)(actor->x[i] + j * 0x10 + 0x10),
                (s16)(actor->y[i] + 0x50),
                getRelocatableHeapBlockBase(gAssetHandles[0x21]),
                0x28,
                0x20,
                0x20,
                0,
                alpha,
                tile
            );
        }

        drawMenuSpriteWithAlphaWideArgs(
            (s16)(actor->x[i] + offset - 8),
            (s16)(actor->y[i] + 0x50),
            getRelocatableHeapBlockBase(gAssetHandles[0x21]),
            0x29,
            0x20,
            0x20,
            0,
            alpha,
            tile
        );

        sprintf(text, D_800E0DB8, i + 1);
        drawMenuAsciiText((s16)(actor->x[i] + 0x34), (s16)(actor->y[i] + 2), text, 0, alpha);

        if (alpha == 0xC0) {
            drawMenuSpriteWithAlphaWideArgs(
                (s16)(actor->x[i] + 2),
                (s16)(actor->y[i] + 0x24),
                getRelocatableHeapBlockBase(gAssetHandles[0x1F]),
                0x90,
                0x20,
                0x20,
                0,
                0xF0,
                0
            );
        }
    }
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/updateCourseSelectPlayerPanels.s")

void initCourseSelectPlayerPanels(CourseSelectPlayerPanelListActor *arg0) {
    if (gPlayerCount == 2) {
        arg0->exitTargetX = 0xA0;
        arg0->x[0] = 0x90;
        arg0->y[0] = -0xC8;
        arg0->x[1] = -0x18C;
        arg0->y[1] = 0x6C;
        arg0->targetX[0] = -0x7E;
        arg0->targetX[1] = -0x7E;
        arg0->deltaX[0] = -1;
        arg0->deltaX[1] = 1;
        arg0->deltaY[0] = 1;
        arg0->deltaY[1] = -1;
        arg0->period[0] = 0x10E;
        arg0->period[1] = 0x10E;
        arg0->speed[0] = 0x68;
        arg0->speed[1] = 0x68;
    } else {
        arg0->x[0] = -0x118;
        arg0->x[1] = -0x118;
        arg0->deltaX[0] = 1;
        arg0->deltaY[0] = 1;
        arg0->deltaX[1] = 1;
        arg0->deltaY[2] = 1;
        arg0->exitTargetX = -0x128;
        arg0->y[0] = -0xC8;
        arg0->y[1] = 0x6C;
        arg0->x[2] = 0x94;
        arg0->y[2] = -0xC8;
        arg0->x[3] = 0x94;
        arg0->y[3] = 0x6C;
        arg0->targetX[0] = -0x88;
        arg0->targetX[1] = -0x88;
        arg0->targetX[2] = 4;
        arg0->targetX[3] = 4;
        arg0->deltaY[1] = -1;
        arg0->deltaX[2] = -1;
        arg0->deltaX[3] = -1;
        arg0->deltaY[3] = -1;
        arg0->period[0] = 0x90;
        arg0->speed[0] = 0x68;
        arg0->period[1] = 0x90;
        arg0->speed[1] = 0x68;
        arg0->period[2] = 0x90;
        arg0->speed[2] = 0x68;
        arg0->period[3] = 0x90;
        arg0->speed[3] = 0x68;
    }
    arg0->timer[1] = 0;
    arg0->state[1] = 0;
    arg0->timer[2] = 0;
    arg0->state[2] = 0;
    arg0->timer[3] = 0;
    arg0->state[3] = 0;
    arg0->timer[0] = 0;
    arg0->state[0] = 0;
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseSelectPlayerPanels);
}

#pragma GLOBAL_ASM("asm.jp/nonmatchings/menu/course_select/course_select_ui/drawCourseSelectCompletePanels.s")

void updateCourseSelectCompletePanels(CourseSelectCompletePanelsActor *actor) {
    CourseSelectAnimatedActor *source0;
    CourseSelectAnimatedActor *source1;
    CourseSelectCompletePanelsActor *task;
    CourseSelectCompletePanelsActor *panel;
    u16 alpha;
    s32 i;

    source0 = gMenuScratch0.coursePreview;
    source1 = gMenuScratch1.coursePreview;
    panel = actor;
    task = panel;
    for (i = 0; i < gPlayerCount; i++) {
        alpha = panel->playerPanelFadeAlpha[i];
        if (alpha == 0) {
            if ((source0 != NULL) && ((source0->transitionState[i] == 4) || (source1->transitionState[i] == 4))) {
                panel->playerPanelFadeAlpha[i] = 1;
            }
        } else {
            if (alpha != 0x100) {
                panel->playerPanelFadeAlpha[i] = alpha + 0x30;
                if (panel->playerPanelFadeAlpha[i] >= 0x100) {
                    panel->playerPanelFadeAlpha[i] = 0x100;
                }
            }
            if (gRacePlayers[i].menuState == 1) {
                panel->playerPanelFadeAlpha[i] = 0;
            }
        }
    }

    if (D_800EC9C0 == 0x10) {
        removeCallbackTask(task);
        for (i = 0; i < gPlayerCount; i++) {
            gRacePlayers[i].menuState = 3;
        }
    } else {
        addRenderCallback(&gMenuRenderCallbackList, (RenderCallback)drawCourseSelectCompletePanels, task);
    }
}

void initCourseSelectCompletePanels(CourseSelectCompletePanelsActor *arg0) {
    s32 var_v0;
    CourseSelectCompletePanelsActor *var_v1;

    if (gPlayerCount == 2) {
        arg0->x = -0x6E;
        arg0->y = -0x4C;
    } else {
        arg0->x = -0x8C;
        arg0->y = -0x46;
    }
    var_v0 = 0;
    var_v1 = arg0;
    if ((s32)gPlayerCount > 0) {
        do {
            var_v1->playerPanelFadeAlpha[0] = 0;
            var_v0++;
            var_v1 = (CourseSelectCompletePanelsActor *)((u8 *)var_v1 + sizeof(s16));
        } while (var_v0 < (s32)gPlayerCount);
    }
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateCourseSelectCompletePanels);
}

void finishCourseSelectUiTask(s32 arg0) {
}
