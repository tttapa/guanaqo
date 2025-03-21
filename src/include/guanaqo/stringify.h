#pragma once

#define GUANAQO_CAT(a, b) GUANAQO_CAT_IMPL(a, b)
#define GUANAQO_CAT_IMPL(a, b) a##b
#define GUANAQO_STRINGIFY(s) GUANAQO_STRINGIFY_IMPL(s)
#define GUANAQO_STRINGIFY_IMPL(s) #s
