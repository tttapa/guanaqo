#include <gtest/gtest.h>

#include <guanaqo/callback-streambuf.hpp>
#include <span>
#include <sstream>

TEST(CallbackStreambuf, simple) {
    std::vector<char> written;
    guanaqo::callback_streambuf sbuf{[&](std::span<const char> data) {
        written.insert(written.end(), data.begin(), data.end());
    }};
    std::ostream os{&sbuf};
    os << "Hello, world" << std::flush;
    EXPECT_EQ(std::string_view{written}, "Hello, world");
    os << 1 << 2 << 3 << 'a' << 'b' << 'c' << std::flush;
    EXPECT_EQ(std::string_view{written}, "Hello, world123abc");
}

TEST(CallbackStreambuf, scoped) {
    std::vector<char> written;
    guanaqo::callback_streambuf sbuf{[&](std::span<const char> data) {
        written.insert(written.end(), data.begin(), data.end());
    }};
    std::ostringstream os;
    {
        guanaqo::scoped_ostream_redirect redirect{os, &sbuf};
        os << "Hello, world";
    }
    os << "Goodbye";
    EXPECT_EQ(std::string_view{written}, "Hello, world");
    EXPECT_EQ(os.str(), "Goodbye");
}
