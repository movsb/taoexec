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

void split_paths(const std::string& pathstr, std::vector<std::string>* paths) {
    paths->clear();

    const char* p = pathstr.c_str();

    while(*p) {
        const char* q = p;
        while(*q && *q != '\n' && *q != '\r')
            ++q;

        // 非空行
        if(q > p)
            paths->push_back(std::string(p, q - p));

        // 非结束，干掉空行
        if(*q == '\n' || *q == '\r')
            ++q;

        p = q;
    }
}

}
}
