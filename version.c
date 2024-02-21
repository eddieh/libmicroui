#include "version.h"

static const char *version = TEMPLATE_VERSION;

const char *
template_version()
{
    return version;
}
