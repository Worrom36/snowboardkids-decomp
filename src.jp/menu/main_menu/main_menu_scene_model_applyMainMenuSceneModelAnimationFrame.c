#include "game/menu/main_menu/main_menu_scene_model.h"

typedef struct MainMenuAnimationWritePart {
    s32 word0;
    s32 word4;
    s32 word8;
    s32 wordC;
    s32 word10;
} MainMenuAnimationWritePart;

void applyMainMenuSceneModelAnimationFrame(MainMenuSceneModel *model) {
    s16 *cursor;
    s32 i;
    MainMenuAnimationWritePart *writePart;
    CharacterModelPart *part;

    cursor = model->animationCursor;
    i = 0;
    writePart = (MainMenuAnimationWritePart *)model;
    do {
        i++;
        writePart[1].word10 = cursor[0] << 11;
        writePart[2].word0 = cursor[1] << 11;
        writePart[2].word4 = cursor[2] << 11;
        writePart++;
        cursor += 3;
    } while (i < MAIN_MENU_SCENE_MODEL_PART_COUNT);

    for (i = 0; i < MAIN_MENU_SCENE_MODEL_PART_COUNT; i++) {
        part = &model->parts[i];
        part->rotation.x = *cursor++;
        part->rotation.y = *cursor++;
        part->rotation.z = *cursor++;
    }

    model->animationCursor = cursor;
}
