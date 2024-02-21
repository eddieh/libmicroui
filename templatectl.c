#include <template.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    fprintf(stderr, "template %s\n", template_version());
    return 0;
}
