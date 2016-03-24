#pragma once

#include <cstring>

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <regex>
#include <functional>

namespace taoexec {
    typedef std::map<std::string, std::string> strstrmap;

    struct __string_nocase_compare {
        bool operator()(const std::string& lhs, const std::string& rhs) {
            return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
        }
    };

    typedef std::map<std::string, std::string, __string_nocase_compare> strstrimap;
}
