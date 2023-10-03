
#ifdef __SWITCH__
#include <switch.h>

void setup_nxlink(void)
{
    socketInitializeDefault();
    nxlinkStdio();
}
#endif