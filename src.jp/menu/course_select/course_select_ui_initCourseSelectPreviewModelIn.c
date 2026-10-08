#include "game/menu/menu_scratch.h"
#include "game/race/race_state.h"
#include "font_encoding.h"
#include "common.h"
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
