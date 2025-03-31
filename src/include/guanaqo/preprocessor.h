#pragma once

// Concatenation helper macros
#define GUANAQO_CONCATENATE_TOKENS_IMPL(a, b) a##b
#define GUANAQO_CONCATENATE_TOKENS(a, b) GUANAQO_CONCATENATE_TOKENS_IMPL(a, b)

// Counting the number of arguments (up to 10 arguments supported here)
#define GUANAQO_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define GUANAQO_NUM_ARGS(...)                                                  \
    GUANAQO_NUM_ARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// Macro to select the correct GUANAQO_JOIN_TOKENS_ARGS_N macro
#define GUANAQO_JOIN_TOKENS_ARGS_CHOOSER(N)                                    \
    GUANAQO_CONCATENATE_TOKENS(GUANAQO_JOIN_TOKENS_ARGS_, N)
#define GUANAQO_JOIN_TOKENS(...)                                               \
    GUANAQO_JOIN_TOKENS_ARGS_CHOOSER(GUANAQO_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

// GUANAQO_JOIN_TOKENS_ARGS_N macros (up to 10 arguments)
// clang-format off
#define GUANAQO_JOIN_TOKENS_ARGS_1(x1) x1
#define GUANAQO_JOIN_TOKENS_ARGS_2(x1, x2) x1##_##x2
#define GUANAQO_JOIN_TOKENS_ARGS_3(x1, x2, x3) x1##_##x2##_##x3
#define GUANAQO_JOIN_TOKENS_ARGS_4(x1, x2, x3, x4) x1##_##x2##_##x3##_##x4
#define GUANAQO_JOIN_TOKENS_ARGS_5(x1, x2, x3, x4, x5) x1##_##x2##_##x3##_##x4##_##x5
#define GUANAQO_JOIN_TOKENS_ARGS_6(x1, x2, x3, x4, x5, x6) x1##_##x2##_##x3##_##x4##_##x5##_##x6
#define GUANAQO_JOIN_TOKENS_ARGS_7(x1, x2, x3, x4, x5, x6, x7) x1##_##x2##_##x3##_##x4##_##x5##_##x6##_##x7
#define GUANAQO_JOIN_TOKENS_ARGS_8(x1, x2, x3, x4, x5, x6, x7, x8) x1##_##x2##_##x3##_##x4##_##x5##_##x6##_##x7##_##x8
#define GUANAQO_JOIN_TOKENS_ARGS_9(x1, x2, x3, x4, x5, x6, x7, x8, x9) x1##_##x2##_##x3##_##x4##_##x5##_##x6##_##x7##_##x8##_##x9
#define GUANAQO_JOIN_TOKENS_ARGS_10(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) x1##_##x2##_##x3##_##x4##_##x5##_##x6##_##x7##_##x8##_##x9##_##x10
// clang-format on

// Macro to select the correct GUANAQO_JOIN_STRINGS_ARGS_N macro
#define GUANAQO_JOIN_STRINGS_ARGS_CHOOSER(N)                                   \
    GUANAQO_CONCATENATE_TOKENS(GUANAQO_JOIN_STRINGS_ARGS_, N)
#define GUANAQO_JOIN_STRINGS(sep, ...)                                         \
    GUANAQO_JOIN_STRINGS_ARGS_CHOOSER(GUANAQO_NUM_ARGS(__VA_ARGS__))(          \
        sep, __VA_ARGS__)

// GUANAQO_JOIN_STRINGS_ARGS_N macros (up to 10 arguments)
// clang-format off
#define GUANAQO_JOIN_STRINGS_ARGS_1(sep, x1) #x1
#define GUANAQO_JOIN_STRINGS_ARGS_2(sep, x1, x2) #x1 sep #x2
#define GUANAQO_JOIN_STRINGS_ARGS_3(sep, x1, x2, x3) #x1 sep #x2 sep #x3
#define GUANAQO_JOIN_STRINGS_ARGS_4(sep, x1, x2, x3, x4) #x1 sep #x2 sep #x3 sep #x4
#define GUANAQO_JOIN_STRINGS_ARGS_5(sep, x1, x2, x3, x4, x5) #x1 sep #x2 sep #x3 sep #x4 sep #x5
#define GUANAQO_JOIN_STRINGS_ARGS_6(sep, x1, x2, x3, x4, x5, x6) #x1 sep #x2 sep #x3 sep #x4 sep #x5 sep #x6
#define GUANAQO_JOIN_STRINGS_ARGS_7(sep, x1, x2, x3, x4, x5, x6, x7) #x1 sep #x2 sep #x3 sep #x4 sep #x5 sep #x6 sep #x7
#define GUANAQO_JOIN_STRINGS_ARGS_8(sep, x1, x2, x3, x4, x5, x6, x7, x8) #x1 sep #x2 sep #x3 sep #x4 sep #x5 sep #x6 sep #x7 sep #x8
#define GUANAQO_JOIN_STRINGS_ARGS_9(sep, x1, x2, x3, x4, x5, x6, x7, x8, x9) #x1 sep #x2 sep #x3 sep #x4 sep #x5 sep #x6 sep #x7 sep #x8 sep #x9
#define GUANAQO_JOIN_STRINGS_ARGS_10(sep, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) #x1 sep #x2 sep #x3 sep #x4 sep #x5 sep #x6 sep #x7 sep #x8 sep #x9 sep #x10
// clang-format on

#define GUANAQO_NOOP()                                                         \
    do {                                                                       \
    } while (false)
