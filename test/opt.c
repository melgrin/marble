#ifndef OPT_SELF_TEST
#define OPT_SELF_TEST 1
#endif

#if OPT_SELF_TEST

#include "../opt.c"
#include <stdio.h>

int main() {
    const char* argv[] = {"opt", "hello", "world", "--width", "1024", "-x", "0", "end"};
    const int argc  = (int) (sizeof(argv) / sizeof(argv[0]));

    uint32_t width = 0;
    bool flag_example = false;
    struct option example_options[] = {
        { "-w", "--width", OPT_UINT32, &width },
        { "-x", "--flag-example", OPT_FLAG, &flag_example },
    };

    int64_t loaded_mask = options_load(argc, argv, sizeof(example_options) / sizeof(example_options[0]), example_options);
    if (loaded_mask > 0) {
        printf("loaded mask = 0x%llx\n", loaded_mask);
        for (int64_t i = 0; i < 64; ++i) {
            if (loaded_mask & (((int64_t) 1) << i)) {
                printf("loaded option at index %lld\n", i);
            }
        }
        printf("width = %u\n", width);
        printf("flag_example = %s\n", flag_example?"true":"false");
        return 0;
    } else {
        printf("loaded mask = %lld\n", loaded_mask);
        return 1;
    }
}

#endif // OPT_SELF_TEST

