#include <template.h>
#include <stdio.h>
#include <string.h>

static struct {
    const char *name;
    const char *input;
    const char *expect;
} test_cases[] = {
    { .name = "Basic test",
      .input = "one 1", .expect = "one 1", },
};

int
main(int argc, char **argv)
{
    const char *name, *input, *expect;
    int test_count, failed, fail_count = 0;

    test_count = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < test_count; i++) {
        name = test_cases[i].name;
        input = test_cases[i].input;
        expect = test_cases[i].expect;

        failed = strcmp(input, expect);

        if (!!failed) {
            fprintf(stderr, "[%s] %s\n\n", "fail", name);
            fprintf(stderr,
                "       input=%s\n"
                "    expected=%s\n", input, expect);
            fail_count++;
        } else
            fprintf(stderr, "[%s] %s\n", "pass", test_cases[i].name);
    }

    fprintf(stderr, "\nRan %d tests, %d failures\n", test_count, fail_count);

    return 0;
}
