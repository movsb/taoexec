#include <cstring>
#include <string>

#include "core.h"
#include "model.h"
#include "view.h"
#include "charset.h"

static void prompt_elevation() {
    if(taoexec::core::is_64bit() && !::IsUserAnAdmin()) {
        auto s = R"(You are running a 64-bit Windows version, but you are NOT running this application)"
            R"( as Administrator. So, actions that need elevation will NOT work.)" "\n"
            R"(If you are experiencing problems, try re-running this application as Administrator, instead.)";
        ::MessageBox(nullptr, s, "", MB_OK|MB_ICONINFORMATION);
    }
}

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
#endif
    taowin::init();
    taoexec::core::init();

    taoexec::core::add_user_variable("myprog", R"(F:\Program Files)");

    taoexec::model::db_t db;
    db.open(taoexec::charset::a2e(R"(taoexec.db)"));

    TW tw(db);
    tw.create();
    tw.show();

    prompt_elevation();

    taowin::loop_message();

    db.close();

    return 0;
}
