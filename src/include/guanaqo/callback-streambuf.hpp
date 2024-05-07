#pragma once

#include <cstring>
#include <functional>
#include <ostream>
#include <span>
#include <streambuf>
#include <vector>

namespace guanaqo {

/// An implementation of a `std::streambuf` that calls the given callback
/// function with the characters that are written.
/// @note   Not thread-safe.
///
/// Inspired by https://github.com/pybind/pybind11/blob/master/include/pybind11/iostream.h
class callback_streambuf : public std::streambuf {
  public:
    using write_func_t = std::function<void(std::span<const char>)>;

  private:
    /// Computes how many bytes at the end of the buffer are part of an
    /// incomplete sequence of UTF-8 bytes.
    /// @pre    `pbase() < pptr()`
    [[nodiscard]] size_t utf8_remainder() const {
        const auto rbase = std::reverse_iterator<char *>(pbase());
        const auto rpptr = std::reverse_iterator<char *>(pptr());
        static auto uch  = [](char c) { return static_cast<unsigned char>(c); };
        auto is_ascii    = [](char c) { return (uch(c) & 0x80) == 0x00; };
        auto is_leading  = [](char c) { return (uch(c) & 0xC0) == 0xC0; };
        auto is_leading_2b = [](char c) { return uch(c) <= 0xDF; };
        auto is_leading_3b = [](char c) { return uch(c) <= 0xEF; };
        // If the last character is ASCII, there are no incomplete code points
        if (is_ascii(*rpptr))
            return 0;
        // Otherwise, work back from the end of the buffer and find the first
        // UTF-8 leading byte
        const auto rpend   = rbase - rpptr >= 3 ? rpptr + 3 : rbase;
        const auto leading = std::find_if(rpptr, rpend, is_leading);
        if (leading == rbase)
            return 0;
        const auto dist  = static_cast<size_t>(leading - rpptr);
        size_t remainder = 0;

        if (dist == 0)
            remainder = 1; // 1-byte code point is impossible
        else if (dist == 1)
            remainder = is_leading_2b(*leading) ? 0 : dist + 1;
        else if (dist == 2)
            remainder = is_leading_3b(*leading) ? 0 : dist + 1;
        // else if (dist >= 3), at least 4 bytes before encountering an UTF-8
        // leading byte, either no remainder or invalid UTF-8.
        // We do not intend to handle invalid UTF-8 here.
        return remainder;
    }

    /// Calls @ref write_func if the buffer is not empty.
    int _sync() {
        if (pbase() != pptr()) { // If buffer is not empty
            // This subtraction cannot be negative, so dropping the sign.
            auto size        = static_cast<size_t>(pptr() - pbase());
            size_t remainder = utf8_remainder();

            if (size > remainder)
                write_func(std::span{pbase(), size - remainder});

            // Copy the remainder at the end of the buffer to the beginning:
            if (remainder > 0)
                std::memmove(pbase(), pptr() - remainder, remainder);
            setp(pbase(), epptr());
            pbump(static_cast<int>(remainder));
        }
        return 0;
    }

    int sync() override { return _sync(); }

    int overflow(int c) override {
        using traits_type = std::streambuf::traits_type;
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            *pptr() = traits_type::to_char_type(c);
            pbump(1);
        }
        return _sync() == 0 ? traits_type::not_eof(c) : traits_type::eof();
    }

  public:
    callback_streambuf(write_func_t write_func, size_t buffer_size = 1024)
        : write_func{std::move(write_func)} {
        buffer.resize(buffer_size);
        setp(buffer.data(), buffer.data() + buffer.size());
    }

    /// Syncs before destroy
    ~callback_streambuf() override { _sync(); }

  private:
    write_func_t write_func;
    std::vector<char> buffer;
};

/// Temporarily replaces the rdbuf of the given ostream. Flushes and restores
/// the old rdbuf upon destruction.
class scoped_ostream_redirect {
  private:
    std::ostream &os;
    std::streambuf *old_buf;

  public:
    explicit scoped_ostream_redirect(std::ostream &os, std::streambuf *rdbuf)
        : os{os}, old_buf{os.rdbuf(rdbuf)} {}

    ~scoped_ostream_redirect() {
        os.flush();
        os.rdbuf(old_buf);
    }

    scoped_ostream_redirect(const scoped_ostream_redirect &) = delete;
    scoped_ostream_redirect(scoped_ostream_redirect &&other) = default;
    scoped_ostream_redirect &
    operator=(const scoped_ostream_redirect &)                     = delete;
    scoped_ostream_redirect &operator=(scoped_ostream_redirect &&) = delete;
};

} // namespace guanaqo
