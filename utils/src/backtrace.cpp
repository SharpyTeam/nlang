#include <utils/macro.hpp>
#include <utils/backtrace.hpp>

#include <stdexcept>

#ifdef NLANG_DEBUG
#include <backward.hpp>
#endif

namespace nlang {

void PrintBacktrace() {
#ifdef NLANG_DEBUG
    using namespace backward;
    StackTrace st; st.load_here(32);
    Printer p;
    p.object = true;
    p.color_mode = ColorMode::always;
    p.address = true;
    p.print(st, stderr);
#else
    throw std::runtime_error("can't print backtrace in release build");
#endif
}

}