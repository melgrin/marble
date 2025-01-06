#ifndef OPT_SELF_TEST
#define OPT_SELF_TEST 1
#endif

#if OPT_SELF_TEST

#include "../opt.c"
#include <stdio.h>

#define Test(condition) _Test(condition, #condition, __FILE__, __LINE__)
static int num_fails = 0;
static bool _Test(bool condition, const char* condition_text, const char* file, int line) {
    printf("%s: '%s' at %s:%d\n", condition ? "PASS" : "FAIL", condition_text, file, line);
    if (!condition) num_fails++;
    return condition;
}

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
        Test(!options_index_was_loaded(0, loaded_mask)); // "opt"
        Test(!options_index_was_loaded(1, loaded_mask)); // "hello"
        Test(!options_index_was_loaded(2, loaded_mask)); // "world"
        Test( options_index_was_loaded(3, loaded_mask)); // "--width"
        Test( options_index_was_loaded(4, loaded_mask)); // "1024"
        Test( options_index_was_loaded(5, loaded_mask)); // "-x"
        Test(!options_index_was_loaded(6, loaded_mask)); // "0"
        Test(!options_index_was_loaded(7, loaded_mask)); // "end"
        return num_fails;
    } else {
        printf("loaded mask = %lld\n", loaded_mask);
        return 1;
    }
}

#endif // OPT_SELF_TEST

