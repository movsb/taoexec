#include <cstring>
#include <string>

#include "core.h"
#include "model.h"
#include "view.h"
#include "charset.h"

#ifdef _DEBUG
int main() {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd) {
#endif
    taowin::init();
    nbsg::core::initialize_globals();

    nbsg::core::add_user_variable("myprog", R"(F:\Program Files)");

    nbsg::model::db_t db;
    db.open(nbsg::charset::a2e(R"(nbsg.db)"));

    TW tw(db);
    tw.create();
    tw.show();

    taowin::loop_message();

    return 0;
}
