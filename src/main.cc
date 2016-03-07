#include <cstring>
#include <string>

#include "core.h"
#include "model.h"
#include "view.h"
#include "charset.h"

static void prompt_elevation() {
    if(taoexec::core::is_wow64() && !::IsUserAnAdmin()) {
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


#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
#endif
    taowin::init();
    taoexec::core::init();

#ifdef TEST
    taoexec::test();
#endif

    taoexec::model::db_t db;
    db.open(taoexec::charset::a2e(R"(taoexec.db)"));

    taoexec::model::item_db_t itemdb;
    itemdb.set_db(*db);

    taoexec::model::config_db_t configdb;
    configdb.set_db(*db);

    taoexec::core::env_var_t env_var;
    env_var.patch(configdb.get("user_vars").append(1, '\0'));
    taoexec::core::add_user_variables(env_var);

    TW& tw = * new TW(itemdb, configdb);
    tw.create();
    tw.show();

    MINI& mini = * new MINI(itemdb, configdb);
    mini.create();
    mini.show();

    prompt_elevation();

    taowin::loop_message();

    db.close();

    return 0;
}
