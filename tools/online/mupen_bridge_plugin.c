/*
 * Mupen64Plus core plugin skeleton for Online Battle.
 *
 * Maps RDRAM symbol gOnlineMailbox <-> localhost UDP / mailbox file consumed
 * by tools/online-client. Build against mupen64plus-core headers and install
 * beside the emulator; until then, use the file-backed Python client alone
 * for lobby/protocol testing.
 *
 * Expected exports (Mupen64Plus plugin ABI):
 *   PluginStartup / PluginShutdown / PluginGetVersion
 *   On a VI interrupt or CoreDoCommand frame hook, call online_bridge_sync().
 */

#include <stdint.h>
#include <string.h>

/* Filled in by the emulator when the plugin resolves the game symbol. */
static uint8_t *g_rdram;
static uint32_t g_mailbox_rdram_offset; /* byte offset of gOnlineMailbox in RDRAM */

#define ONLINE_MAILBOX_SIZE 0x208

void online_bridge_set_rdram(uint8_t *rdram, uint32_t mailbox_offset) {
    g_rdram = rdram;
    g_mailbox_rdram_offset = mailbox_offset;
}

/*
 * Copy RDRAM mailbox out to a host-side buffer (written to the mailbox file or
 * sent over localhost IPC), then copy host inbound fields back into RDRAM.
 */
int online_bridge_sync(const uint8_t *host_inbound, uint8_t *host_outbound) {
    uint8_t *mailbox;

    if (g_rdram == 0 || host_inbound == 0 || host_outbound == 0) {
        return -1;
    }
    mailbox = g_rdram + g_mailbox_rdram_offset;
    memcpy(host_outbound, mailbox, ONLINE_MAILBOX_SIZE);
    /* Preserve game→bridge command; merge bridge→game fields from host. */
    mailbox[0x009] = host_inbound[0x009]; /* bridgeToGameCmd */
    mailbox[0x00A] = host_inbound[0x00A]; /* errorCode */
    memcpy(mailbox + 0x00C, host_inbound + 0x00C, 0x3C); /* lobby */
    memcpy(mailbox + 0x08C, host_inbound + 0x08C, 0x3C * 4); /* remoteIn */
    mailbox[0x17D] = host_inbound[0x17D];
    memcpy(mailbox + 0x1C0, host_inbound + 0x1C0, 8 * 8); /* eventIn */
    memcpy(mailbox + 0x200, host_inbound + 0x200, 4); /* bridgeHeartbeat */
    return 0;
}
