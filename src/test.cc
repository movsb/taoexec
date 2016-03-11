#include "core.h"

#include <iostream>

namespace taoexec {

static void test_get_executor() {
    std::string exts[] = { "", ".txt", ".exe", ".html", ".BAT", ".jpg", ".EXE" , ".md"};
    for (auto& e : exts) {
        std::cout << "get_executor(" << e << "): " << core::get_executor(e) << std::endl;
    }
}

void test() {
    test_get_executor();
}

}
