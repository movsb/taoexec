#pragma once

#include <taowin/tw_taowin.h>

#include "core.h"
#include "model.h"

#include <sstream>
#include <functional>
#include <regex>

class INPUTBOX : public taowin::window_creator {
public:
    typedef std::function<bool(INPUTBOX* that, taowin::control* ctl, const std::string& text)> callback_t;
    INPUTBOX(const std::string& text, callback_t cb) 
        : _cb(cb)
        , _text(std::move(text))
    {}

private:
     callback_t _cb;
     const std::string _text;

protected:
    virtual LPCTSTR get_skin_xml() const override {
        return R"tw(
<window title="input" size="500,400">
    <res>
        <font name="default" face="微软雅黑" size="12"/>
        <font name="consolas" face="Consolas" size="12"/>
    </res>
    <root>
        <vertical>
            <edit name="content" font="consolas" style="multiline,wantreturn,vscroll,hscroll" exstyle="clientedge"/>
            <horizontal height="40" padding="5,5,5,5">
                <control/>
                <button name="ok" text="确定" width="40"/>
                <button name="cancel" text="取消" width="40"/>
        </vertical>
    </root>
</window>
)tw";
    }

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override {
        if(umsg == WM_CREATE) {
            _root->find<taowin::edit>("content")->set_text(_text.c_str());
            return 0;
        }
        return __super::handle_message(umsg, wparam, lparam);
    }

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override {
        if(pc->name() == "ok" || pc->name() == "cancel") {
            if(code == BN_CLICKED) {
                if(_cb(this, pc, _root->find<taowin::edit>("content")->get_text())) {
                    close();
                    return 0;
                }
            }
        }
        return 0;
    }
};

class MINI : public taowin::window_creator {
protected:
    virtual void get_metas(taowin::window::window_meta_t* metas) override {
        __super::get_metas(metas);
        metas->style = WS_POPUP;
        metas->exstyle = WS_EX_TOPMOST | WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_LAYERED;
    }

    virtual LPCTSTR get_skin_xml() const override {
        return R"tw(
<window title="mini" size="80,18">
    <res>
        <font name="default" face="微软雅黑" size="12" />
        <font name="consolas" face="Consolas" size="12" />
    </res>
    <root>
        <vertical padding="">
            <edit name="args" font="consolas" style="border"/>
        </vertical>
    </root>
</window>
)tw";
    }

    virtual bool filter_message(MSG* msg) override {
        if(msg->message == WM_KEYDOWN) {
            switch(msg->wParam) {
            case VK_ESCAPE:
                close();
                return true;
            case VK_RETURN:
            {
                auto edit = _root->find<taowin::edit>("args");
                execute(edit->get_text());
                return true;
            }
            default:
                // I don't want IsDialogMessage to process VK_ESCAPE, because it produces a WM_COMMAND
                // menu message with id == 2. It is undocumented.
                // and, this function call doesn't care the variable _is_dialog.
                if(::IsDialogMessage(_hwnd, msg))
                    return true;
                break;
            }
        }
        return false;
    }

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) {
        switch(umsg) {
        case WM_CREATE:
            // move to top of screen, and set attributes
            ([&]() {
                taowin::rect window_rect = get_window_rect();
                int cx_screen = ::GetSystemMetrics(SM_CXSCREEN);
                ::SetWindowPos(_hwnd, nullptr,
                    (cx_screen - window_rect.width()) / 2, 0, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER);

                ::SetLayeredWindowAttributes(_hwnd, 0, 50, LWA_ALPHA);
            })();

            ::RegisterHotKey(_hwnd, 0, MOD_CONTROL | MOD_SHIFT, 0x5A /* z */);
            _root->find("args")->focus();
            return 0;
        case WM_HOTKEY:
        {
            if(wparam == 0) {
                set_display(2);
            }
            return 0;
        }
        case WM_SETFOCUS:
            if(auto edit = _root->find("args"))
                edit->focus();
            break;
        }
        return __super::handle_message(umsg, wparam, lparam);
    }

    void set_display(int cmd) {
        if(cmd == 2)
            cmd = ::IsWindowVisible(_hwnd) ? 0 : 1;

        if(cmd == 1) {
            if(!::IsWindowVisible(_hwnd))
                ::ShowWindow(_hwnd, SW_SHOW);
            ::SetForegroundWindow(_hwnd);
            ::SetActiveWindow(_hwnd);
            _root->find("args")->focus();
        }
        else {
            ::ShowWindow(_hwnd, SW_HIDE);
            _root->find<taowin::edit>("args")->set_text("");
        }
    }

    virtual void on_final_message() override {
        delete this;
    }

public:
    MINI(nbsg::model::db_t& db)
        : _db(db)
        , _focus(nullptr) {}

private:
    HWND _focus;
    nbsg::model::db_t& _db;

protected:

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) {
        return 0;
    }

    void execute(std::string& args) {
        std::string cmd, env, arg;
        bool is_dir, is_env;
        bool parsed = nbsg::core::parse_args(args, &cmd, &is_env, &env, &is_dir, &arg);
        if(!parsed || cmd.size() == 0) {
            if(args.size()) {
                msgbox("Nothing to do without a cmd specified, correctly specify it.");
                return;
            }
            else {
                set_display(0);
                return;
            }
        }

        auto from_db = [&]()->nbsg::model::item_t* {
            nbsg::model::item_t* found = nullptr;
            std::vector<nbsg::model::item_t*> items;
            int rc = _db.query(cmd, &items);
            if(rc == -1) {
                msgbox("sqlite3 error.");
            } else if(rc == 0) {
                msgbox("Your search `" + cmd + "` does not match anything.");
            } else if(rc == 1) {
                found = items[0];
            } else {
                decltype(items.cbegin()) it;
                for(it = items.cbegin(); it != items.cend(); it++) {
                    if((*it)->index == cmd) {
                        found = *it;
                        break;
                    }
                }

                for(auto pi : items) {
                    if(pi != found)
                        delete pi;
                }

                if(found == nullptr) {
                    msgbox("There are many rows match your given prefix.");
                }
                // found
            }

            return found;
        };

        auto from_env = [&]()->std::string {
            return nbsg::core::which(cmd, env);
        };

        std::string errstr;

        if(is_env) {
            std::string path(is_dir
                ? nbsg::core::which(cmd, env)
                : cmd);

            if(is_dir) {
                if(!path.size()) {
                    msgbox("未找到。");
                    return;
                }

                nbsg::core::explorer(_hwnd, path, [&errstr](const std::string& err) {
                    errstr = err;
                });
            }
            else {
                nbsg::core::execute(_hwnd, path, "", "", "", "", [&errstr](const std::string& err) {
                    errstr = err;
                });
            }
        }
        else {
            auto found = from_db();
            if(!found) {
                msgbox("未找到。");
                return;
            }

            if(is_dir) {
                std::string path = nbsg::core::which(nbsg::core::expand(found->path), "");
                nbsg::core::explorer(_hwnd, path, [&](const std::string& err) {
                    errstr = err;
                });
            }
            else {
                std::string path = nbsg::core::expand(found->path);
                nbsg::core::execute(_hwnd, path, found->params, arg, found->work_dir, found->env, [&](const std::string& err) {
                    errstr = err;
                });
            }

            delete found;
        }

        if(errstr == "ok")
            set_display(0);
        else
            msgbox("失败。");

        return;
    }
};

class ITEM : public taowin::window_creator {
private:
    nbsg::model::db_t&      _db;
    nbsg::model::item_t*    _item;
    std::function<void(nbsg::model::item_t* p)>   _on_succ;

private:
    taowin::edit*   _id;
    taowin::edit*   _index;
    taowin::edit*   _group;
    taowin::edit*   _comment;
    taowin::edit*   _path;
    taowin::edit*   _params;
    taowin::edit*   _work_dir;
    taowin::edit*   _env;
    taowin::edit*   _show;

public:
    ITEM(nbsg::model::db_t& db, nbsg::model::item_t* item, std::function<void(nbsg::model::item_t*)> on_succ = nullptr)
        : _db(db)
        , _item(item)
        , _on_succ(on_succ)
    {
    }

protected:
    virtual LPCTSTR get_skin_xml() const override {
        return R"tw(
<window title="item" size="410,280">
    <res>
        <font name="default" face="微软雅黑" size="12" />
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <vertical height="220">
                <horizontal>
                    <label style="centerimage" text="id" width="70"/>
                    <edit name="id" style="tabstop" exstyle="clientedge" style="disabled"/>
                </horizontal>
                <horizontal>
                    <label style="centerimage" text="index" width="70"/>
                    <edit name="index" style="tabstop" exstyle="clientedge" />
                </horizontal>
                <horizontal>
                    <label style="centerimage" text="group" width="70"/>
                    <edit name="group" style="tabstop" exstyle="clientedge" />
                </horizontal>
                <horizontal>
                    <label style="centerimage" text="comment" width="70"/>
                    <edit name="comment" style="tabstop" exstyle="clientedge" />
                </horizontal>
                <horizontal>
                    <label style="centerimage" text="path" width="70"/>
                    <edit name="path" style="tabstop" exstyle="clientedge" />
                </horizontal>
                <horizontal>
                    <label style="centerimage" text="params" width="70"/>
                    <edit name="params" style="tabstop" exstyle="clientedge" />
                </horizontal>
                <horizontal>
                    <label style="centerimage" text="work_dir" width="70"/>
                    <edit name="work_dir" style="tabstop" exstyle="clientedge" />
                </horizontal>
                <horizontal>
                    <label style="centerimage" text="env" width="70"/>
                    <edit name="env" style="tabstop,disabled" exstyle="clientedge" />
                    <button name="show-env" text="..." width="30"/>
                </horizontal>
                <horizontal>
                    <label style="centerimage" text="show" width="70"/>
                    <edit name="show" style="tabstop" exstyle="clientedge" />
                </horizontal>
            </vertical>
            <control minheight="5"/>
            <horizontal height="40">
                <control/>
                <horizontal width="80">
                    <button name="ok" text="保存"/>
                    <button name="cancel" text="取消"/>
                </horizontal> 
            </horizontal>
        </vertical>
    </root>
</window>

)tw";

    }
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) {
        switch(umsg)
        {
        case WM_CREATE:
        {
            _id         = _root->find<taowin::edit>("id");
            _index      = _root->find<taowin::edit>("index");
            _group      = _root->find<taowin::edit>("group");
            _comment    = _root->find<taowin::edit>("comment");
            _path       = _root->find<taowin::edit>("path");
            _params     = _root->find<taowin::edit>("params");
            _work_dir   = _root->find<taowin::edit>("work_dir");
            _env        = _root->find<taowin::edit>("env");
            _show       = _root->find<taowin::edit>("show");

            if(_item) {
                _id->set_text(_item->id.c_str());
                _index->set_text(_item->index.c_str());
                _group->set_text(_item->group.c_str());
                _comment->set_text(_item->comment.c_str());
                _path->set_text(_item->path.c_str());
                _params->set_text(_item->params.c_str());
                _work_dir->set_text(_item->work_dir.c_str());
                _env->set_text(_item->env.c_str());
                _show->set_text(std::to_string(_item->show).c_str());
            } else {
                _id->set_text("-1");
                _show->set_text("1");
            }

            return 0;
        }
        }

        return __super::handle_message(umsg, wparam, lparam);
    }

    virtual bool filter_message(MSG* msg) override {
        if(msg->message == WM_KEYDOWN) {
            switch(msg->wParam) {
            case VK_ESCAPE:
                close();
                return true;
            case VK_RETURN:
            {
                send_message(WM_COMMAND, MAKEWPARAM(BN_CLICKED, 0), LPARAM(_root->find("ok")->hwnd()));
                return true;
            }
            default:
                // I don't want IsDialogMessage to process VK_ESCAPE, because it produces a WM_COMMAND
                // menu message with id == 2. It is undocumented.
                // and, this function call doesn't care the variable _is_dialog.
                if(::IsDialogMessage(_hwnd, msg))
                    return true;
                break;
            }
        }
        return false;
    }

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) {
        if(pc->name() == "ok") {
            if(code == BN_CLICKED) {
                auto p = new nbsg::model::item_t;
                p->id = _id->get_text();
                p->index = _index->get_text();
                p->group = _group->get_text();
                p->comment = _comment->get_text();
                p->path = _path->get_text();
                p->params = _params->get_text();
                p->work_dir = _work_dir->get_text();
                p->env = _env->get_text();
                p->show = !!std::atoi(_show->get_text().c_str()); // std::stoi will throw

                if(_item == nullptr) {    // create
                    int id;
                    if((id = _db.insert(p)) <= 0) {    // failed
                        msgbox("失败");
                        delete p;
                        return 0;
                    }
                    else {  // succeeded
                        p->id = std::to_string(id);
                        _id->set_text(p->id.c_str());
                        _item = p;
                        _on_succ(p);
                        msgbox("添加成功");
                        return 0;
                    }
                }
                else {      // modify
                    if(_db.modify(p)) {
                        _item->index    = std::move(p->index);
                        _item->group    = std::move(p->group);
                        _item->comment  = std::move(p->comment);
                        _item->path     = std::move(p->path);
                        _item->params   = std::move(p->params);
                        _item->work_dir = std::move(p->work_dir);
                        _item->env      = std::move(p->env);
                        _item->show     = p->show;

                        _on_succ(_item);
                        msgbox("修改成功");
                        delete p;
                        return 0;
                    }
                    else {
                        msgbox("失败");
                        delete p;
                        return 0;
                    }
                }
                close();
                return 0;
            }
        }
        else if(pc->name() == "cancel") {
            if(code == BN_CLICKED) {
                close();
                return 0;
            }
        }
        else if(pc->name() == "show-env") {
            if(code == BN_CLICKED) {
                INPUTBOX input(_env->get_text(), [&](INPUTBOX* that, taowin::control* ctl, const std::string& text) {
                    _root->find<taowin::edit>("env")->set_text(text.c_str());
                    return true;
                });
                input.domodal(this);
                return 0;
            }
        }
        return 0;
    }
};

class TW : public taowin::window_creator {
private:
    nbsg::model::db_t& _db;
    std::vector<nbsg::model::item_t*> _items;

public:
    TW(nbsg::model::db_t& db) 
        : _db(db)
    {
    }

protected:
    virtual LPCTSTR get_skin_xml() const override {
        return R"tw(
<window title="nbsg" size="850,600">
    <res>
        <font name="default" face="微软雅黑" size="12"/>
    </res>
    <root>
        <horizontal padding="5,5,5,5" minheight="150">
            <listview name="list" style="showselalways,ownerdata" exstyle="clientedge" minwidth="300"/>
            <vertical width="80" padding="5,0,0,0">
                <vertical height="100">
                    <button name="refresh" text="刷新"/>
                    <button name="add" text="添加"/>
                    <button name="modify" text="修改" style="disabled"/>
                    <button name="delete" text="删除" style="disabled"/>
                </vertical>
                <control/>
                <button name="mini" text="小窗" height="25"/>
            </vertical>
        </horizontal>
    </root>
</window>
)tw";
    }

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override {
        switch(umsg) {
        case WM_CREATE:
        {
            center();
            auto lv = _root->find<taowin::listview>("list");
            assert(lv);

            struct {
                const char* name;
                int         width;
            } cols[] = {
                {"id",50},
                {"index", 50},
                {"group", 80},
                {"comment", 100},
                {"path", 100},
                {"params", 100},
                {"work_dir", 100},
                {"env", 100},
                {"show", 50},
                {nullptr, 0}
            };

            for(int i = 0; i < _countof(cols); i++) {
                auto col = &cols[i];
                lv->insert_column(col->name, col->width, i);
            }

            // 设置管理员标题
            ([](HWND hwnd) {
                char tt[1024];
                tt[::GetWindowText(hwnd, tt, _countof(tt)-128/* enough */)] = '\0';
                ::strcat(tt, ::IsUserAnAdmin() ? " (Admin)" : " (non-Admin)");
                ::SetWindowText(hwnd, tt);
            })(_hwnd);

            _refresh();

            return 0;
        }
        case WM_CLOSE:
            if(MessageBox(_hwnd, "确认关闭？", "", MB_OKCANCEL) != IDOK) {
                return 0;
            }
        default:
            break;
        }
        return __super::handle_message(umsg, wparam, lparam);
    }

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override {
        if(pc->name() == "list") {
            if(!hdr) return 0;
            taowin::listview* list = static_cast<taowin::listview*>(pc);
            if(code == LVN_GETDISPINFO) {
                NMLVDISPINFO* pdi = reinterpret_cast<NMLVDISPINFO*>(hdr);
                auto rit = _items[pdi->item.iItem]; // right-hand item
                auto lit = &pdi->item;              // left-hand item
                switch(lit->iSubItem) 
                {
                case 0: lit->pszText = (LPSTR)rit->id.c_str(); break;
                case 1: lit->pszText = (LPSTR)rit->index.c_str(); break;
                case 2: lit->pszText = (LPSTR)rit->group.c_str(); break;
                case 3: lit->pszText = (LPSTR)rit->comment.c_str(); break;
                case 4: lit->pszText = (LPSTR)rit->path.c_str(); break;
                case 5: lit->pszText = (LPSTR)rit->params.c_str(); break;
                case 6: lit->pszText = (LPSTR)rit->work_dir.c_str(); break;
                case 7: lit->pszText = (LPSTR)rit->env.c_str(); break;
                case 8: lit->pszText = (LPSTR)(rit->show ? "1" : "0"); break;
                }
            }
            else if(code == LVN_ITEMCHANGED
                || code == LVN_DELETEITEM
                || code == LVN_DELETEALLITEMS
                )
            {
                auto btn_modify = _root->find<taowin::button>("modify");
                auto btn_delete = _root->find<taowin::button>("delete");
                btn_modify->set_enabled(list->get_selected_count() == 1);
                btn_delete->set_enabled(list->get_selected_count() > 0);

                if(code == LVN_DELETEITEM) {
                    auto lv = reinterpret_cast<NMLISTVIEW*>(hdr);
                    auto it = _items.begin() + lv->iItem;   // TODO make post
                    _items.erase(it);
                } else if(code == LVN_DELETEALLITEMS) {
                    // never happens.
                }

                return 0;
            }
            else if(code == NM_DBLCLK) {
                auto item = reinterpret_cast<NMITEMACTIVATE*>(hdr);
                _execute(item->iItem);
                return 0;
            }
        }
        else if(pc->name() == "add" || pc->name() == "modify") {
            if(code != BN_CLICKED) 
                return 0;
            if(pc->name() == "add") {
                taowin::listview* lv = _root->find<taowin::listview>("list");
                // callback
                auto on_added = [&](nbsg::model::item_t* p) {
                    if(_items.size()==0 || _items.back() != p)
                        _items.push_back(p);
                    int count = _items.size();
                    lv->set_item_count(count, LVSICF_NOINVALIDATEALL);
                    lv->redraw_items(count - 1, count - 1);
                };

                ITEM item(_db, nullptr, on_added);
                item.domodal(this);
                return 0;
            }
            else if(pc->name() == "modify") {
                taowin::listview* lv = _root->find<taowin::listview>("list");
                int lvid = lv->get_next_item(-1, LVNI_SELECTED);
                if(lvid == -1) return 0;

                nbsg::model::item_t& it = *_items[lvid];

                // callback
                auto on_modified = [&](nbsg::model::item_t* p) {
                    lv->redraw_items(lvid, lvid);
                };

                ITEM item(_db, &it, on_modified);
                item.domodal(this);

                return 0;
            }
        }
        else if(pc->name() == "delete") {
            if(code != BN_CLICKED)
                return 0;

            auto list = static_cast<taowin::listview*>(_root->find("list"));
            int count = list->get_selected_count();
            if(msgbox(("确定要删除选中的 " + std::to_string(count) + "项？").c_str(), MB_OKCANCEL) == IDOK) {
                int index = -1;
                std::vector<int> selected;
                while((index = list->get_next_item(index, LVNI_SELECTED)) != -1) {
                    selected.push_back(index);
                }

                for(auto it = selected.crbegin(); it != selected.crend(); it++) {
                    int id = std::atoi(_items[*it]->id.c_str());
                    if(_db.remove(id)) {
                        if(list->delete_item(*it)) {
                            continue;
                        }
                    }
                    msgbox("删除失败！");
                    break;
                }
            }
            return 0;
        }
        else if(pc->name() == "refresh") {
            _refresh();
            return 0;
        }
        else if(pc->name() == "mini") {
            MINI* mini = new MINI(_db);
            mini->create();
            mini->show();
            return 0;
        }
        return 0;
    }

private:
    void _refresh() {
        _db.query("", &_items);

        taowin::listview* lv = _root->find<taowin::listview>("list");
        lv->set_item_count(_items.size(), 0);   // cause invalidate all.
    }

    void _execute(int i) {
        if(i<0 || i>(int)_items.size() - 1)
            return;

        auto& path      = _items[i]->path;
        auto& params    = _items[i]->params;
        auto& wd        = _items[i]->work_dir;
        auto& env       = _items[i]->env;

        nbsg::core::execute(_hwnd, path, params, "", wd, env, [&](const std::string& err) {
            if(err != "ok") {
                msgbox("失败。", MB_ICONEXCLAMATION);
            }
        });

    }
};
