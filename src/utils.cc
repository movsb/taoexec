#include "utils.h"

#include <algorithm>
#include <windows.h>

namespace taoexec {
namespace utils {

std::string tolower(const std::string& raw) {
    std::string t(raw);
    std::transform(t.begin(), t.end(), t.begin(), ::tolower);
    return t;
}

const char* char_next(const char* s) {
    return ::CharNext(s);
}

}
}
