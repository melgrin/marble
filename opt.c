#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

typedef enum option_type {
    OPT_FLAG = 1,
    OPT_UINT32,
} option_type;

typedef struct option {
    const char* short_name;
    const char* long_name;
    enum option_type type;
    void* value;
} option;

int64_t options_load(const int argc, const char** argv, const int options_count, struct option* options) {
    if (argc > 64) return -1; // need room in loaded_mask

    int64_t loaded_mask = 0;

    for (int i = 0; i < argc; ++i) {
        for (int j = 0; j < options_count; ++j) {
            struct option* opt = &options[j];
            if ((opt->short_name && 0 == strcmp(argv[i], opt->short_name)) ||
                (opt->long_name && 0 == strcmp(argv[i], opt->long_name))) {
                if (opt->type == OPT_FLAG) {
                    *(bool*)opt->value = true;
                    loaded_mask |= 1 << i;
                } else {
                    if (i+1 == argc) {
                        printf("option %s requires an argument\n", argv[i]);
                        return -1;
                    }
                    loaded_mask |= 1 << i;
                    i += 1;
                    const char* arg = argv[i];
                    char* end;
                    errno = 0;
                    assert(opt->type == OPT_UINT32); // TODO
                    unsigned long int value = strtoul(arg, &end, 10);
                    if (value == 0) return -1;
                    if (errno != 0) return errno;
                    static_assert(sizeof(unsigned long) == sizeof(uint32_t), "non-64-bit platform? unimplemented");
                    *(uint32_t*)opt->value = value;
                    loaded_mask |= 1 << i;
                }
            }
        }
    }

    return loaded_mask;
}

bool options_index_was_loaded(int64_t i, int64_t loaded_mask) {
    if (i >= 64) return false;
    return (loaded_mask & (((int64_t) 1) << i));
}

// TODO
// other option types (int16, float32, etc)
// error reporting abstraction to avoid printf (or at least -D option for it)
// maybe make loaded_mask zero always if failed to load
// hex input
