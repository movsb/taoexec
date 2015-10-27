#pragma once

#include <taowin/tw_taowin.h>

#include "core.h"
#include "model.h"

#include <sstream>
#include <functional>

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
<window title="item" size="400,300">
    <res>
        <font name="default" face="微软雅黑" size="12" />
    </res>
    <root>
        <vertical padding="5,5,5,5">
            <horizontal width="300" height="230">
                <vertical width="70">
                    <label style="centerimage" text="ID"/>
                    <label style="centerimage" text="index"/>
                    <label style="centerimage" text="group"/>
                    <label style="centerimage" text="comment"/>
                    <label style="centerimage" text="path"/>
                    <label style="centerimage" text="params"/>
                    <label style="centerimage" text="work_dir"/>
                    <label style="centerimage" text="env"/>
                    <label style="centerimage" text="show"/>
                </vertical>
                <vertical >
                    <edit name="id" exstyle="clientedge" style="disabled"/>
                    <edit name="index" exstyle="clientedge" />
                    <edit name="group" exstyle="clientedge" />
                    <edit name="comment" exstyle="clientedge" />
                    <edit name="path" exstyle="clientedge" />
                    <edit name="params" exstyle="clientedge" />
                    <edit name="work_dir" exstyle="clientedge" />
                    <edit name="env" exstyle="clientedge" />
                    <edit name="show" exstyle="clientedge" />
                </vertical>
                <vertical padding="10,0,0,10" width="80" height="60">
                    <button name="ok" text="保存"/>
                    <button name="cancel" text="取消"/>
                </vertical> 
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
<window title="nbsg" size="700,500">
    <res>
        <font name="default" face="微软雅黑" size="12"/>
    </res>
    <root>
        <horizontal padding="5,5,5,5">
            <listview name="list" style="showselalways,ownerdata" exstyle="clientedge" minwidth="300"/>
            <vertical padding="5,0,0,0" width="80" height="100">
                <button name="refresh" text="刷新"/>
                <button name="add" text="添加"/>
                <button name="modify" text="修改" style="disabled"/>
                <button name="delete" text="删除" style="disabled"/>
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
                {"path_expanded", 100},
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
                case 5: lit->pszText = (LPSTR)rit->path_expanded.c_str(); break;
                case 6: lit->pszText = (LPSTR)rit->params.c_str(); break;
                case 7: lit->pszText = (LPSTR)rit->work_dir.c_str(); break;
                case 8: lit->pszText = (LPSTR)rit->env.c_str(); break;
                case 9: lit->pszText = (LPSTR)(rit->show ? "1" : "0"); break;
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
                    if(_items.size() && _items.back() != p)
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
                    it.path_expanded = nbsg::core::expand(it.path.c_str());
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
        return 0;
    }

private:
    void _refresh() {
        _db.query("", &_items);

        for(auto pi : _items) {
            pi->path_expanded = nbsg::core::expand(pi->path); // std::move used :-)
        }

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

        ::STARTUPINFO si = {sizeof(si)};
        ::PROCESS_INFORMATION pi;

        std::string cmdline = '"' + path + "\" " + params;
        const char* cd = wd.size() ? wd.c_str() : nullptr;
        if(::CreateProcess(nullptr, (char*)cmdline.c_str(),
            nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, cd,
            &si, &pi))
        {
            ::CloseHandle(pi.hThread);
            ::CloseHandle(pi.hProcess);
        }
        else {
            msgbox("失败");
        }
    }
};
