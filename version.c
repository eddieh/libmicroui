#include "version.h"

static const char *version = MICROUI_VERSION;

const char *
mu_version()
{
    return version;
}
