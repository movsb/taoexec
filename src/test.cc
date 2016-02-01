#include "core.h"

#include <iostream>

namespace taoexec {

static void test_get_executer() {
    std::string exts[] = { "", ".txt", ".exe", ".html", ".BAT", ".jpg", ".EXE" };
    for (auto& e : exts) {
        std::cout << "get_executer(" << e << "): " << core::get_executer(e) << std::endl;
    }
}

void test() {
    test_get_executer();
}

}
