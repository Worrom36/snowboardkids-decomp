/* Capture AI DMA PCM from an LLE RSP; no host audio device or timing throttle. */
#include <m64p_plugin.h>
#include <m64p_common.h>
#include <stdio.h>

#define RAM_BYTES 0x400000U

static AUDIO_INFO audio;
static FILE *capture;
static unsigned int clockRate = 48681812;
static unsigned int sampleRate;
static int captureError;

static void writeWord(unsigned int value) {
    unsigned char bytes[4] = { value >> 24, value >> 16, value >> 8, value };
    if (fwrite(bytes, 1, sizeof(bytes), capture) != sizeof(bytes)) captureError = 1;
}

int SetCapturePath(const char *path) {
    capture = fopen(path, "wb");
    return capture != NULL;
}

int GetCaptureError(void) { return captureError; }

m64p_error PluginStartup(m64p_dynlib_handle core, void *context, void (*debug)(void *, int, const char *)) {
    return M64ERR_SUCCESS;
}

m64p_error PluginShutdown(void) {
    if (capture && fclose(capture)) captureError = 1;
    capture = NULL;
    return captureError ? M64ERR_FILES : M64ERR_SUCCESS;
}

m64p_error PluginGetVersion(m64p_plugin_type *type, int *version, int *api, const char **name, int *caps) {
    if (type) *type = M64PLUGIN_AUDIO;
    if (version) *version = 0x10000;
    if (api) *api = 0x020000;
    if (name) *name = "Snowboard Kids AI PCM capture";
    if (caps) *caps = 0;
    return M64ERR_SUCCESS;
}

int InitiateAudio(AUDIO_INFO info) { audio = info; return 1; }
int RomOpen(void) { return 1; }
void RomClosed(void) {}

void AiDacrateChanged(int system) {
    clockRate = system == SYSTEM_PAL ? 49656530 : system == SYSTEM_MPAL ? 48628316 : 48681812;
    sampleRate = clockRate / ((*audio.AI_DACRATE_REG & 0x3fff) + 1);
}

void AiLenChanged(void) {
    unsigned int address = *audio.AI_DRAM_ADDR_REG & 0xfffff8;
    unsigned int length = *audio.AI_LEN_REG & 0x3fff8;
    unsigned int i;
    unsigned char bytes[4];
    if (!length) return;
    if (!capture || !sampleRate || address > RAM_BYTES || length > RAM_BYTES-address) {
        captureError = 1;
        return;
    }
    writeWord(sampleRate);
    writeWord(length);
    /* Mupen's RDRAM words use host order. Store canonical N64 big-endian PCM. */
    for (i = 0; i < length; i += 4) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        bytes[0] = audio.RDRAM[address+i+3]; bytes[1] = audio.RDRAM[address+i+2];
        bytes[2] = audio.RDRAM[address+i+1]; bytes[3] = audio.RDRAM[address+i];
#else
        bytes[0] = audio.RDRAM[address+i]; bytes[1] = audio.RDRAM[address+i+1];
        bytes[2] = audio.RDRAM[address+i+2]; bytes[3] = audio.RDRAM[address+i+3];
#endif
        if (fwrite(bytes, 1, 4, capture) != 4) captureError = 1;
    }
}

void ProcessAList(void) { captureError = 2; } /* Requires an LLE RSP, not an HLE callback. */
void SetSpeedFactor(int percent) {}
void VolumeUp(void) {}
void VolumeDown(void) {}
int VolumeGetLevel(void) { return 100; }
void VolumeSetLevel(int level) {}
void VolumeMute(void) {}
const char *VolumeGetString(void) { return "PCM capture"; }
