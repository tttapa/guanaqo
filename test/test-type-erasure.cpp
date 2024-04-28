#include <gtest/gtest.h>

#include <guanaqo/type-erasure.hpp>

#include <cstdio>
#include <cstring>
#include <memory>
#include <string_view>

#define PF() (std::printf("%s: %s\n", __PRETTY_FUNCTION__, msg))

struct Noisy {
    static unsigned created;
    static unsigned destroyed;
    Noisy(const char *msg = "default") {
        std::strcpy(this->msg, msg);
        PF();
        ++created;
    }
    Noisy(const Noisy &other) {
        std::strcpy(this->msg, other.msg);
        PF();
        ++created;
    }
    Noisy(Noisy &&other) {
        std::strcpy(this->msg, other.msg);
        std::strcpy(other.msg, "moved-from");
        PF();
        ++created;
    }
    Noisy &operator=(const Noisy &other) {
        std::strcpy(this->msg, other.msg);
        return PF(), *this;
    }
    Noisy &operator=(Noisy &&other) {
        std::strcpy(this->msg, other.msg);
        std::strcpy(other.msg, "moved-from");
        return PF(), *this;
    }
    ~Noisy() {
        PF();
        ++destroyed;
    }
    const char *get_msg() const { return PF(), msg; }
    void set_msg(const char *m) {
        PF();
        std::strcpy(msg, m);
    }
    char msg[16]{};
};

struct CustomVTable : guanaqo::BasicVTable {
    const char *(*get_msg)(const void *)  = nullptr;
    void (*set_msg)(void *, const char *) = nullptr;
    CustomVTable()                        = default;
    template <class T>
    CustomVTable(std::in_place_t, T &t) : BasicVTable{std::in_place, t} {
        get_msg = guanaqo::type_erased_wrapped<T, &T::get_msg>();
        set_msg = guanaqo::type_erased_wrapped<T, &T::set_msg>();
    }
};

template <class Alloc = std::allocator<std::byte>>
struct CustomTypeErased : guanaqo::TypeErased<CustomVTable, Alloc, 0> {
    using TypeErased = guanaqo::TypeErased<CustomVTable, Alloc, 0>;
    using TypeErased::self;
    using TypeErased::TypeErased;
    using TypeErased::vtable;
    using typename TypeErased::allocator_type;
    template <class T, class... Args>
    static CustomTypeErased make(Args &&...args) {
        return TypeErased::template make<CustomTypeErased, T>(
            std::forward<Args>(args)...);
    }
    const char *get_msg() const { return vtable.get_msg(self); }
    void set_msg(const char *m) { vtable.set_msg(self, m); }
};

template <class Alloc = std::allocator<std::byte>>
struct CustomTypeErasedBigBuf
    : guanaqo::TypeErased<CustomVTable, Alloc, 256> {
    using TypeErased = guanaqo::TypeErased<CustomVTable, Alloc, 256>;
    using TypeErased::self;
    using TypeErased::TypeErased;
    using TypeErased::vtable;
    using typename TypeErased::allocator_type;
    template <class T, class... Args>
    static CustomTypeErasedBigBuf make(Args &&...args) {
        return TypeErased::template make<CustomTypeErasedBigBuf, T>(
            std::forward<Args>(args)...);
    }
    const char *get_msg() const { return vtable.get_msg(self); }
    void set_msg(const char *m) { vtable.set_msg(self, m); }
};

unsigned Noisy::created   = 0;
unsigned Noisy::destroyed = 0;

#include <array>
#include <cctype>
#include <iomanip>
#include <memory_resource>
#include <vector>

using pmr_alloc = std::pmr::polymorphic_allocator<std::byte>;
using PMRCTE    = CustomTypeErased<pmr_alloc>;

void dump_buf(const auto &buffer) {
    constexpr size_t per_row = 16;
    size_t rows              = (buffer.size() + per_row - 1) / per_row;
    for (size_t r = 0; r < rows; ++r) {
        size_t rr   = r * per_row;
        size_t cols = std::min(per_row, buffer.size() - rr);
        std::cout << std::hex << std::setw(4) << rr << std::dec << ": ";
        for (size_t c = 0; c < cols; ++c) {
            auto ch = static_cast<uint8_t>(buffer[rr + c]);
            std::cout << std::hex << std::setw(2) << +ch << ' ' << std::dec;
        }
        std::cout << "  ";
        for (size_t c = 0; c < cols; ++c) {
            auto ch = static_cast<char>(buffer[rr + c]);
            if (std::isprint(ch)) {
                std::cout << "  " << ch;
            } else {
                std::cout << "  ·";
            }
        }
        std::cout << '\n';
    }
}

static_assert(std::uses_allocator_v<PMRCTE, pmr_alloc>);

TEST(TypeErasure, TypeErased) {
    std::array<std::byte, 256> buffer{};
    {
        std::pmr::monotonic_buffer_resource mbr{
            buffer.data(), buffer.size(), std::pmr::null_memory_resource()};
        std::cout << "default: " << std::pmr::get_default_resource() << '\n';
        std::cout << "mbr:     " << &mbr << '\n';
        PMRCTE n{std::allocator_arg, &mbr, Noisy{"Message"}};
        dump_buf(buffer);
        EXPECT_STREQ(n.get_msg(), "Message");
        EXPECT_TRUE(n);
        PMRCTE n2{std::move(n), &mbr};
        dump_buf(buffer);
        EXPECT_FALSE(n);
        EXPECT_TRUE(n2);
        EXPECT_STREQ(n2.get_msg(), "Message");
        PMRCTE n3{std::allocator_arg, &mbr};
        EXPECT_FALSE(n3);
        dump_buf(buffer);
        n3 = n2;
        EXPECT_TRUE(n2);
        EXPECT_TRUE(n3);
        dump_buf(buffer);
        EXPECT_STREQ(n2.get_msg(), "Message");
        EXPECT_STREQ(n3.get_msg(), "Message");
        n3.set_msg("foo");
        EXPECT_STREQ(n2.get_msg(), "Message");
        EXPECT_STREQ(n3.get_msg(), "foo");
        std::cout << sizeof(PMRCTE) << "\n";
        std::pmr::vector<PMRCTE> v{&mbr};
        v.reserve(2);
        dump_buf(buffer);
        v.emplace_back<Noisy>("vec 0");
        dump_buf(buffer);
        EXPECT_STREQ(v.back().get_msg(), "vec 0");
        v.emplace_back<Noisy>("vec 1");
        dump_buf(buffer);
        EXPECT_STREQ(v.front().get_msg(), "vec 0");
        EXPECT_STREQ(v.back().get_msg(), "vec 1");
    }
    std::cout << "Created:   " << Noisy::created << "\n"
              << "Destroyed: " << Noisy::destroyed << "\n";
    EXPECT_EQ(Noisy::created, Noisy::destroyed);
    dump_buf(buffer);
    auto buf_str = std::string_view{
        reinterpret_cast<const char *>(buffer.data()), buffer.size()};
    using namespace std::string_view_literals;
    EXPECT_NE(buf_str.find("Message"sv), buf_str.npos);
    EXPECT_NE(buf_str.find("foo\0age"sv), buf_str.npos);
    EXPECT_NE(buf_str.find("vec 0"sv), buf_str.npos);
    EXPECT_NE(buf_str.find("vec 1"sv), buf_str.npos);
}

TEST(TypeErasure, allocatorAware) {
    std::array<std::byte, 1024> buffer{};
    std::pmr::monotonic_buffer_resource mbr{buffer.data(), buffer.size(),
                                            std::pmr::null_memory_resource()};
    // Stand-alone make function with allocator argument
    auto a = PMRCTE::make<Noisy>(std::allocator_arg, &mbr, "make-pmr");
    ASSERT_TRUE(a);
    EXPECT_STREQ(a.get_msg(), "make-pmr");
    EXPECT_EQ(a.get_allocator(), pmr_alloc{&mbr});

    // As part of allocator aware container.
    std::pmr::vector<PMRCTE> v{&mbr};
    v.reserve(2);
    v.emplace_back<Noisy>("noisy-a");
    v.emplace_back<Noisy>("noisy-b");
    ASSERT_TRUE(v[0]);
    ASSERT_TRUE(v[1]);
    EXPECT_EQ(v[0].get_allocator(), pmr_alloc{&mbr});
    EXPECT_EQ(v[1].get_allocator(), pmr_alloc{&mbr});
    EXPECT_STREQ(v[0].get_msg(), "noisy-a");
    EXPECT_STREQ(v[1].get_msg(), "noisy-b");
    std::pmr::vector<PMRCTE> v_copy{v, &mbr}; // pmr doesn't propagate allocator
    ASSERT_TRUE(v_copy[0]);
    ASSERT_TRUE(v_copy[1]);
    EXPECT_EQ(v_copy[0].get_allocator(), pmr_alloc{&mbr});
    EXPECT_EQ(v_copy[1].get_allocator(), pmr_alloc{&mbr});
    EXPECT_STREQ(v_copy[0].get_msg(), "noisy-a");
    EXPECT_STREQ(v_copy[1].get_msg(), "noisy-b");
    auto v_move = std::move(v); // pmr moves allocator
    ASSERT_TRUE(v_move[0]);
    ASSERT_TRUE(v_move[1]);
    EXPECT_EQ(v_move[0].get_allocator(), pmr_alloc{&mbr});
    EXPECT_EQ(v_move[1].get_allocator(), pmr_alloc{&mbr});
    EXPECT_STREQ(v_move[0].get_msg(), "noisy-a");
    EXPECT_STREQ(v_move[1].get_msg(), "noisy-b");
}

// TODO: add test where Noisy itself is also allocator aware

TEST(TypeErasure, moveDifferentAllocators) {
    std::array<std::byte, 256> buffer_1{};
    std::pmr::monotonic_buffer_resource mbr_1{buffer_1.data(), buffer_1.size(),
                                              std::pmr::null_memory_resource()};
    std::array<std::byte, 256> buffer_2{};
    std::pmr::monotonic_buffer_resource mbr_2{buffer_2.data(), buffer_2.size(),
                                              std::pmr::null_memory_resource()};
    auto a = PMRCTE(std::allocator_arg, &mbr_1, Noisy{"noisy-a"});
    auto b = PMRCTE(std::allocator_arg, &mbr_2);
    ASSERT_TRUE(a);
    EXPECT_FALSE(b);
    EXPECT_EQ(b.get_allocator(), pmr_alloc{&mbr_2});
    b = std::move(a);
    EXPECT_FALSE(std::allocator_traits<
                 pmr_alloc>::propagate_on_container_move_assignment::value);
    EXPECT_EQ(b.get_allocator(), pmr_alloc{&mbr_2});
    EXPECT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(b.get_msg(), "noisy-a");
    b.set_msg("noisy-b");
    EXPECT_STREQ(b.get_msg(), "noisy-b");
}

TEST(TypeErasure, moveConstructDifferentAllocators) {
    std::array<std::byte, 256> buffer_1{};
    std::pmr::monotonic_buffer_resource mbr_1{buffer_1.data(), buffer_1.size(),
                                              std::pmr::null_memory_resource()};
    std::array<std::byte, 256> buffer_2{};
    std::pmr::monotonic_buffer_resource mbr_2{buffer_2.data(), buffer_2.size(),
                                              std::pmr::null_memory_resource()};
    auto a = PMRCTE(std::allocator_arg, &mbr_1, Noisy{"noisy-a"});
    ASSERT_TRUE(a);
    auto b = PMRCTE(std::move(a), &mbr_2);
    EXPECT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_EQ(b.get_allocator(), pmr_alloc{&mbr_2});
    EXPECT_STREQ(b.get_msg(), "noisy-a");
    b.set_msg("noisy-b");
    EXPECT_STREQ(b.get_msg(), "noisy-b");
}

TEST(TypeErasure, moveConstructDifferentAllocatorsEmpty) {
    std::array<std::byte, 256> buffer_1{};
    std::pmr::monotonic_buffer_resource mbr_1{buffer_1.data(), buffer_1.size(),
                                              std::pmr::null_memory_resource()};
    std::array<std::byte, 256> buffer_2{};
    std::pmr::monotonic_buffer_resource mbr_2{buffer_2.data(), buffer_2.size(),
                                              std::pmr::null_memory_resource()};
    auto a = PMRCTE(std::allocator_arg, &mbr_1);
    EXPECT_FALSE(a);
    auto b = PMRCTE(std::move(a), &mbr_2);
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
    EXPECT_EQ(b.get_allocator(), pmr_alloc{&mbr_2});
}

// Parametrized test fixture.
template <class>
struct TypeErasedTest : public testing::Test {};
TYPED_TEST_SUITE_P(TypeErasedTest);

TYPED_TEST_P(TypeErasedTest, as) {
    auto a = TypeParam::template make<Noisy>("test-as");
    EXPECT_STREQ(a.get_msg(), "test-as");
    a.template as<Noisy>().set_msg("a-replace");
    EXPECT_STREQ(a.get_msg(), "a-replace");
}

TYPED_TEST_P(TypeErasedTest, getPtr) {
    auto a = TypeParam::template make<Noisy>("test-ptr");
    EXPECT_EQ(a.get_pointer(), &a.template as<Noisy>());
}

TYPED_TEST_P(TypeErasedTest, getConstPtr) {
    auto a = TypeParam::template make<Noisy>("test-ptr");
    EXPECT_EQ(a.get_const_pointer(), &a.template as<Noisy>());
}

TYPED_TEST_P(TypeErasedTest, getPtrPtr) {
    Noisy n{"test-ptr"};
    auto a = TypeParam{&n};
    EXPECT_EQ(a.get_pointer(), &a.template as<Noisy>());
}

TYPED_TEST_P(TypeErasedTest, getConstPtrConstPtr) {
    Noisy n{"test-ptr"};
    auto a = TypeParam{&std::as_const(n)};
    EXPECT_EQ(a.get_const_pointer(), &a.template as<const Noisy>());
}

TYPED_TEST_P(TypeErasedTest, getPtrConstPtrWrong) {
    Noisy n{"test-ptr"};
    auto a = TypeParam{&std::as_const(n)};
    EXPECT_THROW(static_cast<void>(a.get_pointer()),
                 guanaqo::bad_type_erased_constness);
}

TYPED_TEST_P(TypeErasedTest, asWrongType) {
    auto a = TypeParam::template make<Noisy>("test-as");
    EXPECT_THROW(static_cast<void>(a.template as<std::vector<Noisy>>()),
                 guanaqo::bad_type_erased_type);
}

TYPED_TEST_P(TypeErasedTest, asWrongConst) {
    Noisy n{"test-ptr"};
    auto a = TypeParam{&std::as_const(n)};
    EXPECT_THROW(static_cast<void>(a.template as<Noisy>()),
                 guanaqo::bad_type_erased_constness);
}

TYPED_TEST_P(TypeErasedTest, asConst) {
    Noisy n{"test-ptr"};
    auto a = TypeParam{&n};
    EXPECT_EQ(a.get_const_pointer(), &a.template as<const Noisy>());
}

TYPED_TEST_P(TypeErasedTest, copyConstruct) {
    auto a = TypeParam::template make<Noisy>("test-copy");
    auto b{a};
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(a.get_msg(), "test-copy");
    EXPECT_STREQ(b.get_msg(), "test-copy");
    a.set_msg("a-replace");
    EXPECT_STREQ(a.get_msg(), "a-replace");
    EXPECT_STREQ(b.get_msg(), "test-copy");
    b.set_msg("b-replace");
    EXPECT_STREQ(a.get_msg(), "a-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, copyConstructPtr) {
    Noisy n{"test-copy"};
    auto a = TypeParam::template make<Noisy *>(&n);
    auto b{a};
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    EXPECT_FALSE(a.owns_referenced_object());
    EXPECT_FALSE(b.owns_referenced_object());
    EXPECT_STREQ(n.get_msg(), "test-copy");
    EXPECT_STREQ(a.get_msg(), "test-copy");
    EXPECT_STREQ(b.get_msg(), "test-copy");
    a.set_msg("a-replace");
    EXPECT_STREQ(n.get_msg(), "a-replace");
    EXPECT_STREQ(a.get_msg(), "a-replace");
    EXPECT_STREQ(b.get_msg(), "a-replace");
    b.set_msg("b-replace");
    EXPECT_STREQ(n.get_msg(), "b-replace");
    EXPECT_STREQ(a.get_msg(), "b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, copyConstructPtrConst) {
    Noisy n{"test-copy"};
    auto a = TypeParam::template make<const Noisy *>(&n);
    auto b{a};
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    EXPECT_FALSE(a.owns_referenced_object());
    EXPECT_FALSE(b.owns_referenced_object());
    EXPECT_STREQ(n.get_msg(), "test-copy");
    EXPECT_STREQ(a.get_msg(), "test-copy");
    EXPECT_STREQ(b.get_msg(), "test-copy");
    EXPECT_THROW(a.set_msg("a-fail"), guanaqo::bad_type_erased_constness);
    EXPECT_THROW(b.set_msg("b-fail"), guanaqo::bad_type_erased_constness);
}

TYPED_TEST_P(TypeErasedTest, copy) {
    auto a = TypeParam::template make<Noisy>("a-copy");
    auto b = TypeParam::template make<Noisy>("b-copy");
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(a.get_msg(), "a-copy");
    EXPECT_STREQ(b.get_msg(), "b-copy");
    b = a;
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(a.get_msg(), "a-copy");
    EXPECT_STREQ(b.get_msg(), "a-copy");
    a.set_msg("a-replace");
    EXPECT_STREQ(a.get_msg(), "a-replace");
    EXPECT_STREQ(b.get_msg(), "a-copy");
    b.set_msg("b-replace");
    EXPECT_STREQ(a.get_msg(), "a-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, copyPtr) {
    Noisy n{"a-copy"};
    auto a = TypeParam::template make<Noisy *>(&n);
    auto b = TypeParam::template make<Noisy>("b-copy");
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    EXPECT_FALSE(a.owns_referenced_object());
    EXPECT_TRUE(b.owns_referenced_object());
    EXPECT_STREQ(a.get_msg(), "a-copy");
    EXPECT_STREQ(b.get_msg(), "b-copy");
    b = a;
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    EXPECT_FALSE(a.owns_referenced_object());
    EXPECT_FALSE(b.owns_referenced_object());
    EXPECT_STREQ(a.get_msg(), "a-copy");
    EXPECT_STREQ(b.get_msg(), "a-copy");
    a.set_msg("a-replace");
    EXPECT_STREQ(a.get_msg(), "a-replace");
    EXPECT_STREQ(b.get_msg(), "a-replace");
    b.set_msg("b-replace");
    EXPECT_STREQ(a.get_msg(), "b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, copyFromEmpty) {
    auto a = TypeParam();
    auto b = TypeParam::template make<Noisy>();
    EXPECT_TRUE(b);
    b = a;
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TYPED_TEST_P(TypeErasedTest, copyToEmpty) {
    auto a = TypeParam();
    auto b = TypeParam::template make<Noisy>("test-copy");
    EXPECT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(b.get_msg(), "test-copy");
    a = b;
    EXPECT_STREQ(a.get_msg(), "test-copy");
    EXPECT_STREQ(b.get_msg(), "test-copy");
    a.set_msg("a-replace");
    EXPECT_STREQ(a.get_msg(), "a-replace");
    EXPECT_STREQ(b.get_msg(), "test-copy");
    b.set_msg("b-replace");
    EXPECT_STREQ(a.get_msg(), "a-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, copySelf) {
    auto a = TypeParam::template make<Noisy>("test-copy");
    ASSERT_TRUE(a);
    EXPECT_STREQ(a.get_msg(), "test-copy");
    a = a;
    ASSERT_TRUE(a);
    EXPECT_STREQ(a.get_msg(), "test-copy");
}

TYPED_TEST_P(TypeErasedTest, copyEmptyToEmpty) {
    auto a = TypeParam();
    auto b = TypeParam();
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
    a = b;
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TYPED_TEST_P(TypeErasedTest, copyEmptySelf) {
    auto a = TypeParam();
    a      = a;
    EXPECT_FALSE(a);
}

TYPED_TEST_P(TypeErasedTest, copyConstructFromEmpty) {
    auto a = TypeParam();
    auto b{a};
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TYPED_TEST_P(TypeErasedTest, moveConstruct) {
    auto a = TypeParam::template make<Noisy>("test-move");
    auto b{std::move(a)};
    ASSERT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(b.get_msg(), "test-move");
    b.set_msg("b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, moveConstructPtr) {
    Noisy n{"test-move"};
    auto a = TypeParam::template make<Noisy *>(&n);
    auto b{std::move(a)};
    ASSERT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(n.get_msg(), "test-move");
    EXPECT_STREQ(b.get_msg(), "test-move");
    b.set_msg("b-replace");
    EXPECT_STREQ(n.get_msg(), "b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, move) {
    auto a = TypeParam::template make<Noisy>("a-move");
    auto b = TypeParam::template make<Noisy>("b-move");
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    b = std::move(a);
    ASSERT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(b.get_msg(), "a-move");
    b.set_msg("b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, movePtr) {
    Noisy n{"test-move"};
    auto a = TypeParam::template make<Noisy *>(&n);
    auto b = TypeParam::template make<Noisy>("b-move");
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    b = std::move(a);
    ASSERT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(n.get_msg(), "test-move");
    EXPECT_STREQ(b.get_msg(), "test-move");
    b.set_msg("b-replace");
    EXPECT_STREQ(n.get_msg(), "b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, moveFromEmpty) {
    auto a = TypeParam();
    auto b = TypeParam::template make<Noisy>();
    EXPECT_FALSE(a);
    EXPECT_TRUE(b);
    b = std::move(a);
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TYPED_TEST_P(TypeErasedTest, moveToEmpty) {
    auto a = TypeParam();
    auto b = TypeParam::template make<Noisy>("test-move");
    EXPECT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(b.get_msg(), "test-move");
    a = std::move(b);
    EXPECT_TRUE(a);
    EXPECT_FALSE(b);
}

TYPED_TEST_P(TypeErasedTest, moveSelf) {
    auto a = TypeParam::template make<Noisy>("test-move");
    ASSERT_TRUE(a);
    EXPECT_STREQ(a.get_msg(), "test-move");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
    a = std::move(a);
#pragma GCC diagnostic pop
    ASSERT_TRUE(a);
    EXPECT_STREQ(a.get_msg(), "test-move");
}

TYPED_TEST_P(TypeErasedTest, moveEmptyToEmpty) {
    auto a = TypeParam();
    auto b = TypeParam();
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
    a = std::move(b);
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TYPED_TEST_P(TypeErasedTest, moveConstructFromEmpty) {
    auto a = TypeParam();
    auto b{std::move(a)};
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TYPED_TEST_P(TypeErasedTest, moveConstructAllocAware) {
    auto a = TypeParam::template make<Noisy>("test-move");
    TypeParam b{std::move(a), std::allocator<std::byte>()};
    ASSERT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(b.get_msg(), "test-move");
    b.set_msg("b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, moveConstructAllocAwarePtr) {
    Noisy n{"test-move"};
    auto a = TypeParam::template make<Noisy *>(&n);
    TypeParam b{std::move(a), std::allocator<std::byte>()};
    ASSERT_FALSE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(n.get_msg(), "test-move");
    EXPECT_STREQ(b.get_msg(), "test-move");
    b.set_msg("b-replace");
    EXPECT_STREQ(n.get_msg(), "b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, moveConstructFromEmptyAllocAware) {
    auto a = TypeParam();
    TypeParam b{std::move(a), std::allocator<std::byte>()};
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TYPED_TEST_P(TypeErasedTest, copyConstructAllocAware) {
    auto a = TypeParam::template make<Noisy>("test-copy");
    TypeParam b{a, std::allocator<std::byte>()};
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(a.get_msg(), "test-copy");
    EXPECT_STREQ(b.get_msg(), "test-copy");
    b.set_msg("b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, copyConstructAllocAwarePtr) {
    Noisy n{"test-copy"};
    auto a = TypeParam::template make<Noisy *>(&n);
    TypeParam b{a, std::allocator<std::byte>()};
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    EXPECT_STREQ(n.get_msg(), "test-copy");
    EXPECT_STREQ(a.get_msg(), "test-copy");
    EXPECT_STREQ(b.get_msg(), "test-copy");
    b.set_msg("b-replace");
    EXPECT_STREQ(n.get_msg(), "b-replace");
    EXPECT_STREQ(a.get_msg(), "b-replace");
    EXPECT_STREQ(b.get_msg(), "b-replace");
}

TYPED_TEST_P(TypeErasedTest, copyConstructFromEmptyAllocAware) {
    auto a = TypeParam();
    TypeParam b{a, std::allocator<std::byte>()};
    EXPECT_FALSE(a);
    EXPECT_FALSE(b);
}

TYPED_TEST_P(TypeErasedTest, throwingCopyCtor) {
    struct test_exception {};
    struct Throwing : Noisy {
        using Noisy::Noisy;
        Throwing(const Throwing &o) : Noisy{o} { throw test_exception(); }
    };
    auto a = TypeParam::template make<Throwing>();
    EXPECT_THROW({ auto b{a}; }, test_exception);
    auto c = TypeParam();
    EXPECT_THROW({ c = a; }, test_exception);
}

TYPED_TEST_P(TypeErasedTest, throwingCtor) {
    struct test_exception {};
    struct Throwing : Noisy {
        Throwing() { throw test_exception(); }
    };
    EXPECT_THROW(TypeParam::template make<Throwing>(), test_exception);
}

REGISTER_TYPED_TEST_SUITE_P(
    TypeErasedTest, as, getPtr, getConstPtr, getPtrPtr, getConstPtrConstPtr,
    getPtrConstPtrWrong, asWrongType, asWrongConst, asConst, copyConstruct,
    copyConstructPtr, copyConstructPtrConst, copy, copyPtr, copyFromEmpty,
    copyToEmpty, copySelf, copyEmptyToEmpty, copyEmptySelf,
    copyConstructFromEmpty, moveConstruct, moveConstructPtr, move, movePtr,
    moveFromEmpty, moveToEmpty, moveSelf, moveEmptyToEmpty,
    moveConstructFromEmpty, moveConstructAllocAware, moveConstructAllocAwarePtr,
    moveConstructFromEmptyAllocAware, copyConstructAllocAware,
    copyConstructAllocAwarePtr, copyConstructFromEmptyAllocAware,
    throwingCopyCtor, throwingCtor);
struct TENoBuffer : CustomTypeErased<> {
    using CustomTypeErased<>::CustomTypeErased;
    template <class T, class... Args>
    static TENoBuffer make(Args &&...args) {
        return TypeErased::template make<TENoBuffer, T>(
            std::forward<Args>(args)...);
    }
};
struct TEBigBuffer : CustomTypeErasedBigBuf<> {
    using CustomTypeErasedBigBuf<>::CustomTypeErasedBigBuf;
    template <class T, class... Args>
    static TEBigBuffer make(Args &&...args) {
        return TypeErased::template make<TEBigBuffer, T>(
            std::forward<Args>(args)...);
    }
};
using Configs = testing::Types<TENoBuffer, TEBigBuffer>;
INSTANTIATE_TYPED_TEST_SUITE_P(TypeErasure, TypeErasedTest, Configs, );