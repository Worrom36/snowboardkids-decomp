/* Deterministic virtual controllers for the optional Mupen64Plus runtime tests. */
#include <m64p_plugin.h>
#include <m64p_common.h>

static BUTTONS inputs[4];
static int playerCount = 1;

m64p_error PluginStartup(m64p_dynlib_handle core, void *context, void (*debug)(void *, int, const char *)) {
    return M64ERR_SUCCESS;
}

m64p_error PluginShutdown(void) {
    return M64ERR_SUCCESS;
}

m64p_error PluginGetVersion(m64p_plugin_type *type, int *version, int *api, const char **name, int *caps) {
    if (type) *type = M64PLUGIN_INPUT;
    if (version) *version = 0x10000;
    if (api) *api = 0x020100;
    if (name) *name = "Snowboard Kids deterministic test controllers";
    if (caps) *caps = 0;
    return M64ERR_SUCCESS;
}

void SetButtons(int player, unsigned int value) {
    if (player >= 0 && player < 4) inputs[player].Value = value;
}

void SetPlayerCount(int count) {
    if (count >= 1 && count <= 4) playerCount = count;
}

void InitiateControllers(CONTROL_INFO info) {
    int i;
    for (i = 0; i < 4; i++) {
        info.Controls[i].Present = i < playerCount;
        info.Controls[i].RawData = 0;
        info.Controls[i].Plugin = PLUGIN_MEMPAK;
        info.Controls[i].Type = CONT_TYPE_STANDARD;
    }
}

void GetKeys(int player, BUTTONS *keys) {
    *keys = inputs[player];
}

int RomOpen(void) { return 1; }
void RomClosed(void) {}
void SDL_KeyDown(int mod, int key) {}
void SDL_KeyUp(int mod, int key) {}
void ControllerCommand(int player, unsigned char *command) {}
void ReadController(int player, unsigned char *command) {}
