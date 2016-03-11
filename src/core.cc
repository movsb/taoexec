#include "core.h"
#include "shell.h"
#include "utils.h"

namespace taoexec {
namespace core {

std::map<std::string, std::string>  g_executor;

std::string get_executor(const std::string& ext) {
    std::string cmd;
    std::string lower_ext(utils::tolower(ext));

    if (!lower_ext.size())
        return cmd;

    // hit from cache
    if (g_executor.count(lower_ext))
        return g_executor[lower_ext];

    std::string what_file = shell::query_registry(HKEY_CLASSES_ROOT, ext, "");
    if (what_file.size()) {
        cmd = shell::query_registry(HKEY_CLASSES_ROOT, what_file + R"(\shell\open\command)", "");
        if (cmd.size()) {

        }
    }

    // make cache
    g_executor[lower_ext] = cmd;
    return cmd;
}

}
}
