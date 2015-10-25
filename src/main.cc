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

// taoweb from here

taoweb::http::error_page_t error_page;

static int                                  threads_created;
static taoweb::lock_count                   threads;
static taoweb::lock_queue<taoweb::client_t> clients;

unsigned int __stdcall handler_thread(void* ud) {
    taoweb::client_t client;
    while((true)) {
        threads.inc();
        client = clients.pop();
        threads.dec();

        // handler here
        taoweb::http::http_header_t header;
        header.read_headers(client.fd);
        if(!header._header_lines.size())
            continue;

        nbsg::view::handler_t handler(client, header);
        handler.set_db(g_pdb);
        handler.handle();
    }

    return 0;
}

void create_worker_thread(taoweb::client_t& client) {
    clients.push(client);
    if(threads.size() < 3) {
        HANDLE thr = (HANDLE)_beginthreadex(NULL, 0, handler_thread, NULL, 0, NULL);
        CloseHandle(thr);
        std::cout << "threads created: " << threads_created << ", threads spare: " << threads.size() << std::endl;
    }
}

int main2() {
    using namespace taoweb;

    win_sock _wsa;

    socket_server server("127.0.0.1", 8080, 64);

    server.start();

    client_t client;
    while(server.accept(&client)) {
        create_worker_thread(client);
    }

    return 0;
}


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
    
    main2();

    return 0;
}
