#include "common.h"
#include "PR/os_convert.h"
#include "game/engine/asset_manager.h"
#include "game/engine/render_callback.h"
#include "game/engine/callback_task_scheduler.h"
#include "game/engine/controller_input.h"
#include "game/engine/relocatable_heap.h"
#include "game/engine/system_runtime.h"
#include "game/menu/renderer/menu_renderer.h"
#include "game/menu/renderer/menu_render_utils.h"
#include "game/menu/renderer/sprite_palette_relocation.h"

void initMenuSpriteActorDebugControls(MenuRenderSpriteActor *arg0) {
    setCallbackTaskCallback(arg0, (CallbackTaskCallback)updateMenuSpriteActorDebugControls);
}
