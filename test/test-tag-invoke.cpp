#include <guanaqo/tag-invoke.hpp>
#include <gtest/gtest.h>
#include <cstdio>

struct operation_fn {
    template <class Op>
    // The operation type should opt in to the tag to provide a custom
    // implementation.
        requires(guanaqo::tag_invocable<operation_fn, Op, int>)
    auto operator()(Op op, int i) const
        noexcept(guanaqo::is_nothrow_tag_invocable_v<operation_fn, Op, int>) {
        return guanaqo::guanaqo_tag_invoke(*this, op, i);
    }

    template <class Op>
    // Fallback implementation for unknown operations
        requires(!guanaqo::tag_invocable<operation_fn, Op, int>)
    auto operator()(Op, int i) const {
        return i;
    }
} inline constexpr operation;

struct Multiply {
    int factor;
};

struct Unknown {};

int guanaqo_tag_invoke(guanaqo::tag_t<operation>, Multiply m, int i) {
    return i * m.factor;
}

namespace guanaqo_test {

struct MultiplyFriend {
    int factor;

    friend int guanaqo_tag_invoke(guanaqo::tag_t<operation>, MultiplyFriend m,
                                  int i) {
        std::puts(__PRETTY_FUNCTION__);
        return i * m.factor;
    }
};

} // namespace guanaqo_test

TEST(TagInvoke, basic) {
    Multiply times2{2};
    EXPECT_EQ(operation(times2, 42), 84);
    Unknown uk;
    EXPECT_EQ(operation(uk, 42), 42);
    guanaqo_test::MultiplyFriend times3{3};
    EXPECT_EQ(operation(times3, 42), 126);
}
