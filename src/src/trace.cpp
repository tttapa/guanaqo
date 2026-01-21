#include <guanaqo/trace.hpp>

namespace guanaqo {

GUANAQO_EXPORT TraceLogger &get_trace_logger() {
    static TraceLogger instance{16'384};
    return instance;
}

} // namespace guanaqo
