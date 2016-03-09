#pragma once

#include <string>
#include <vector>

namespace taoexec {
namespace utils {

std::string tolower(const std::string& raw);
const char* char_next(const char* s);
void split_paths(const std::string& pathstr, std::vector<std::string>* paths);

}
}
