#include <cctype>
#include <cstring>
#include <process.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>

#include "core.h"
#include "model.h"
#include "view.h"
#include "charset.h"


nbsg::model::db_t* g_pdb;

int main() {
    const char* paths[] = {
        "${exe_dir}",
        "${windows}",
        "${cd}",
        "${system}",
        "${appdata}",
        "${home}",
        "${program}",
        "${program_x86}",
        "${desktop}",
        "${windows}\\explorer.exe",
        "$reg(HKLM,Software\\Vim\\Gvim,path,64)",
        "$env(USERPROFILE)",
        "$app_path(firefox.exe)",
    };

    nbsg::core::initialize_globals();

    nbsg::core::add_user_variable("myprog", R"(F:\Program Files)");

    for (auto& path : paths) {
        std::cout << nbsg::core::expand(path) << std::endl;
    };

    nbsg::model::db_t db;
    db.open(nbsg::charset::a2e(R"(ÖÐÎÄ.db)"));

    g_pdb = &db;
    
    return 0;
}
