#include <cstring>
#include <string>

#include "event.h"
#include "exec.h"
#include "model.h"
#include "view.h"
#include "charset.h"

static void prompt_elevation() {
    if(taoexec::shell::is_wow64() && !::IsUserAnAdmin()) {
        auto s = R"(You are running a 64-bit Windows version, but you are NOT running this application)"
            R"( as Administrator. So, actions that need elevation will NOT work.)" "\n"
            R"(If you are experiencing problems, try re-running this application as Administrator, instead.)";
        ::MessageBox(GetActiveWindow(), s, "", MB_OK|MB_ICONINFORMATION);
    }
}

#define  TEST

#ifdef TEST
    namespace taoexec {
        extern void test();
    }
#endif

#ifdef __PR__
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif

    taowin::init();

#ifdef TEST
    taoexec::test();
#endif

    taoexec::eventx::event_manager_t evtmgr;
    _evtmgr = &evtmgr;

    taoexec::model::db_t db;
    db.open(taoexec::charset::a2e(R"(taoexec.db)"));

    taoexec::model::config_db_t configdb;
    configdb.set_db(*db);

    taoexec::model::item_db_t itemdb;
    itemdb.set_db(*db);
    itemdb.set_fuzzy_search(configdb.get("fuzzy_search", "1") == "1");

    taoexec::exec::executor_manager_t exec_mgr;
    exec_mgr._itemdb = &itemdb;
    exec_mgr._cfgdb = &configdb;
    exec_mgr.init();

    _evtmgr->attach("main:new", [&](taoexec::eventx::event_args_i* __args) {
        taoexec::view::TW& tw = *new taoexec::view::TW(itemdb, configdb);
        tw.create();
        tw.show();
        return true;
    });

    taoexec::view::MINI& mini = * new taoexec::view::MINI(itemdb, configdb);
    mini.create();
    mini.show();

    prompt_elevation();

    taowin::loop_message();

    db.close();

    return 0;
}
