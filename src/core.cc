#include "core.h"
#include "shell.h"
#include "utils.h"

namespace taoexec {
namespace core {

std::map<std::string, std::string>  g_executer;

std::string get_executer(const std::string& ext) {
    std::string cmd;
    std::string lower_ext(utils::tolower(ext));

    if (!lower_ext.size())
        return cmd;

    // hit from cache
    if (g_executer.count(lower_ext))
        return g_executer[lower_ext];

    std::string what_file = shell::query_registry(HKEY_CLASSES_ROOT, ext, "");
    if (what_file.size()) {
        cmd = shell::query_registry(HKEY_CLASSES_ROOT, what_file + R"(\shell\open\command)", "");
        if (cmd.size()) {

        }
    }

    // make cache
    g_executer[lower_ext] = cmd;
    return cmd;
}

}
}
