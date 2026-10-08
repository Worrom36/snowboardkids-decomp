#include "PRinternal/piint.h"
#include "PR/ultraerror.h"

s32 __osEPiRawReadIo(OSPiHandle *pihandle, u32 devAddr, u32 *data) {
    register u32 stat;

#ifdef _DEBUG
    if (devAddr & 0x3) {
        __osError(ERR_OSPIRAWREADIO, 1, devAddr);
        return -1;
    }
#endif

    WAIT_ON_IOBUSY(stat);
    *data = IO_READ(pihandle->baseAddress | devAddr);

    return 0;
}
