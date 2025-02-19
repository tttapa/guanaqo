#include <gtest/gtest.h>

#include <guanaqo/any-ptr.hpp>

#include <any>
#include <memory>
#include <utility>

TEST(AnyPtr, nonConst) {
    struct S {
        int i;
    };
    S s{42};
    guanaqo::any_ptr p{&s};
    ASSERT_TRUE(p.has_value());
    ASSERT_FALSE(p.is_immutable());
    ASSERT_EQ(p.cast<S>(), &s);
    ASSERT_EQ(p.cast<S>()->i, 42);
    p.cast<S>()->i = 84;
    ASSERT_EQ(s.i, 84);
}

TEST(AnyPtr, Const) {
    struct S {
        int i;
    };
    const S s{42};
    guanaqo::any_ptr p{&s};
    ASSERT_TRUE(p.has_value());
    ASSERT_TRUE(p.is_immutable());
    ASSERT_EQ(p.cast<const S>(), &s);
    ASSERT_EQ(p.cast<const S>()->i, 42);
}

TEST(AnyPtr, asConst) {
    struct S {
        int i;
    };
    S s{42};
    guanaqo::any_ptr p{&std::as_const(s)};
    ASSERT_TRUE(p.has_value());
    ASSERT_TRUE(p.is_immutable());
    ASSERT_EQ(p.cast<const S>(), &s);
    ASSERT_EQ(p.cast<const S>()->i, 42);
    s.i = 84;
    ASSERT_EQ(p.cast<const S>()->i, 84);
}

TEST(AnyPtr, constCastNonConst) {
    struct S {
        int i;
    };
    S s{42};
    guanaqo::any_ptr p{&std::as_const(s)};
    ASSERT_TRUE(p.has_value());
    ASSERT_TRUE(p.is_immutable());
    ASSERT_EQ(p.cast<const S>(), &s);
    ASSERT_THROW(p.cast<S>(), std::bad_any_cast);
}

TEST(AnyPtr, castWrongType) {
    struct S {
        int i;
    };
    struct T {};
    S s{42};
    guanaqo::any_ptr p{&s};
    ASSERT_THROW(p.cast<T>(), std::bad_any_cast);
}

TEST(SharedAnyPtr, nonConst) {
    struct S {
        int i;
    };
    auto s = std::make_shared<S>(42);
    guanaqo::shared_any_ptr p{s};
    ASSERT_TRUE(p.has_value());
    ASSERT_FALSE(p.is_immutable());
    ASSERT_EQ(p.cast<S>(), s.get());
    ASSERT_EQ(p.cast<S>()->i, 42);
    p.cast<S>()->i = 84;
    ASSERT_EQ(s->i, 84);
}

TEST(SharedAnyPtr, move) {
    struct S {
        int i;
    };
    auto s = std::make_shared<S>(42);
    guanaqo::shared_any_ptr p{s};
    guanaqo::shared_any_ptr q = std::move(p);
    ASSERT_FALSE(p.has_value());
    ASSERT_TRUE(q.has_value());
    ASSERT_FALSE(q.is_immutable());
    ASSERT_EQ(q.cast<S>(), s.get());
    ASSERT_EQ(q.cast<S>()->i, 42);
    q.cast<S>()->i = 21;
    ASSERT_EQ(s->i, 21);
    ASSERT_EQ(q.cast<S>()->i, 21);
}

TEST(SharedAnyPtr, copy) {
    struct S {
        int i;
    };
    auto s = std::make_shared<S>(42);
    guanaqo::shared_any_ptr p{s};
    guanaqo::shared_any_ptr q = p; // NOLINT(*-unnecessary-copy-*)
    ASSERT_TRUE(p.has_value());
    ASSERT_FALSE(p.is_immutable());
    ASSERT_TRUE(q.has_value());
    ASSERT_FALSE(q.is_immutable());
    ASSERT_EQ(p.cast<S>(), s.get());
    ASSERT_EQ(q.cast<S>(), s.get());
    ASSERT_EQ(p.cast<S>()->i, 42);
    ASSERT_EQ(q.cast<S>()->i, 42);
    p.cast<S>()->i = 84;
    ASSERT_EQ(s->i, 84);
    ASSERT_EQ(p.cast<S>()->i, 84);
    ASSERT_EQ(q.cast<S>()->i, 84);
    q.cast<S>()->i = 21;
    ASSERT_EQ(s->i, 21);
    ASSERT_EQ(p.cast<S>()->i, 21);
    ASSERT_EQ(q.cast<S>()->i, 21);
}

TEST(SharedAnyPtr, Const) {
    struct S {
        int i;
    };
    auto s = std::make_shared<const S>(42);
    guanaqo::shared_any_ptr p{s};
    ASSERT_TRUE(p.has_value());
    ASSERT_TRUE(p.is_immutable());
    ASSERT_EQ(p.cast<const S>(), s.get());
    ASSERT_EQ(p.cast<const S>()->i, 42);
}

TEST(SharedAnyPtr, asConst) {
    struct S {
        int i;
    };
    auto s = std::make_shared<S>(42);
    guanaqo::shared_any_ptr p{static_pointer_cast<const S>(s)};
    ASSERT_TRUE(p.has_value());
    ASSERT_TRUE(p.is_immutable());
    ASSERT_EQ(p.cast<const S>(), s.get());
    ASSERT_EQ(p.cast<const S>()->i, 42);
    s->i = 84;
    ASSERT_EQ(p.cast<const S>()->i, 84);
}

TEST(SharedAnyPtr, constCastNonConst) {
    struct S {
        int i;
    };
    auto s = std::make_shared<S>(42);
    guanaqo::shared_any_ptr p{static_pointer_cast<const S>(s)};
    ASSERT_TRUE(p.has_value());
    ASSERT_TRUE(p.is_immutable());
    ASSERT_EQ(p.cast<const S>(), s.get());
    ASSERT_THROW(p.cast<S>(), std::bad_any_cast);
}

TEST(SharedAnyPtr, castWrongType) {
    struct S {
        int i;
    };
    struct T {};
    auto s = std::make_shared<S>(42);
    guanaqo::shared_any_ptr p{s};
    ASSERT_THROW(p.cast<T>(), std::bad_any_cast);
}
