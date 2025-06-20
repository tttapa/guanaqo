namespace guanaqo {

enum class LoopDir : bool {
    Forward,
    Backward,
};

template <class I>
[[gnu::always_inline]] inline void
foreach_chunked(I i_begin, I i_end, auto chunk_size, auto func_chunk,
                auto func_rem, LoopDir dir = LoopDir::Forward) {
    if (dir == LoopDir::Forward) {
        I i;
        for (i = i_begin; i + chunk_size <= i_end; i += chunk_size)
            func_chunk(i);
        I rem_i = i_end - i;
        if (rem_i > 0)
            func_rem(i, rem_i);
    } else {
        I rem_i = (i_end - i_begin) % chunk_size;
        I i     = i_end - rem_i;
        if (rem_i > 0)
            func_rem(i, rem_i);
        for (i -= chunk_size; i >= i_begin; i -= chunk_size)
            func_chunk(i);
    }
}

template <class I>
[[gnu::always_inline]] inline void
foreach_chunked_merged(I i_begin, I i_end, auto chunk_size, auto func_chunk,
                       LoopDir dir = LoopDir::Forward) {
    if (dir == LoopDir::Forward) {
        I i;
        for (i = i_begin; i + chunk_size <= i_end; i += chunk_size)
            func_chunk(i, chunk_size);
        I rem_i = i_end - i;
        if (rem_i > 0)
            func_chunk(i, rem_i);
    } else {
        I rem_i = (i_end - i_begin) % chunk_size;
        I i     = i_end - rem_i;
        if (rem_i > 0)
            func_chunk(i, rem_i);
        for (i -= chunk_size; i >= i_begin; i -= chunk_size)
            func_chunk(i, chunk_size);
    }
}

} // namespace guanaqo
