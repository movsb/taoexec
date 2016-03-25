#pragma once

#include <taowin/tw_taowin.h>

#include "exec.h"
#include "model.h"
#include "shell.h"
#include "utils.h"

#include <sstream>
#include <functional>
#include <regex>
#include <shlwapi.h>

namespace taoexec {

namespace view {

class INPUTBOX : public taowin::window_creator {
public:
    typedef std::function<bool(INPUTBOX* that, taowin::control* ctl, const std::string& text)> callback_t;
    INPUTBOX(const std::string& text, callback_t cb)
        : _cb(cb)
        , _text(std::move(text)) {
    }

private:
    callback_t _cb;
    const std::string _text;

protected:
    virtual LPCTSTR get_skin_xml() const override;

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override;
};

class ITEM : public taowin::window_creator {
private:
    taoexec::model::item_db_t& _db;
    taoexec::model::item_t*    _item;
    std::function<void(taoexec::model::item_t* p)>          _on_succ;
    std::function<std::string(const std::string& field)>    _on_init;

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
    ITEM(taoexec::model::item_db_t& db,
        taoexec::model::item_t* item,
        std::function<void(taoexec::model::item_t*)> on_succ = nullptr,
        std::function<std::string(const std::string& field)> on_init = nullptr)
        : _db(db)
        , _item(item)
        , _on_succ(on_succ)
        , _on_init(on_init) 
    {
    }

protected:
    virtual LPCTSTR get_skin_xml() const override;
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);
    virtual bool filter_message(MSG* msg) override;
    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr);
};

class CONFIG : public taowin::window_creator {
private:
    using config_db_t = taoexec::model::config_db_t;

    config_db_t&                        _cfg;
    std::vector<config_db_t::item_t*>   _items;

    HMENU                               _hmenu;

    enum class menuid {
        none,
        add,
        modify,
        remove,
    };

    class INPUT : public taowin::window_creator {
    public:
        using callback_t = std::function<bool(const std::string& name, const std::string& value, const std::string& comment)>;

    protected:
        const config_db_t::item_t* _item;


        callback_t _onok;
        //callback_t _oncancel;

    protected:
        virtual void get_metas(taowin::window::window_meta_t* metas) override {
            __super::get_metas(metas);
        }

        virtual LPCTSTR get_skin_xml() const override;

        virtual bool filter_message(MSG* msg) override;

        virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);

    public:
        INPUT(const config_db_t::item_t* item = nullptr, callback_t onok = nullptr)
            : _item(item)
            , _onok(onok)
            //, _oncancel(nullptr)
        {
        }

    protected:
        virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr);

        virtual void on_final_message() {
            __super::on_final_message();
        }
    };

public:
    CONFIG(taoexec::model::config_db_t& cfg)
        : _cfg(cfg)
        , _hmenu(nullptr)
    {

    }

protected:
    virtual LPCTSTR get_skin_xml() const override;

    menuid show_menu();

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);

    virtual bool filter_message(MSG* msg) override;

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr);

    virtual void on_final_message() override {
        __super::on_final_message();
        delete this;
    }
};

class MINI : public taowin::window_creator {
protected:
    virtual void get_metas(taowin::window::window_meta_t* metas) override {
        __super::get_metas(metas);
        metas->style = WS_POPUP;
        metas->exstyle = WS_EX_TOPMOST | WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_LAYERED;
    }

    virtual LPCTSTR get_skin_xml() const override;

    virtual bool filter_message(MSG* msg) override;

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);

    void set_display(int cmd);

    virtual void on_final_message() override {
        delete this;
    }

public:
    MINI(taoexec::model::item_db_t& db, taoexec::model::config_db_t& cfg)
        : _db(db)
        , _cfg(cfg)
        , _focus(nullptr) 
        , _p_registry_executor(nullptr)
    {

    }

    ~MINI() {
        for (auto& it : _commanders)
            delete it.second;

        delete _p_registry_executor;
    }

private:
    HWND _focus;
    taoexec::model::item_db_t& _db;
    taoexec::model::config_db_t& _cfg;

private:
    class command_executor_i;
    std::map <std::string, command_executor_i*> _commanders;

    class registry_executor;
    registry_executor* _p_registry_executor;






protected:

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) {
        return 0;
    }

    // 命令行支持的模式：
    /*
     *  [commander] [:] [command-specs]
     **/
    void execute(std::string& __args);
};

class TW : public taowin::window_creator {
private:
    taoexec::model::item_db_t& _db;
    taoexec::model::config_db_t& _cfg;
    std::vector<taoexec::model::item_t*> _items;

public:
    TW(taoexec::model::item_db_t& db, taoexec::model::config_db_t& cfg) 
        : _db(db)
        , _cfg(cfg)
    {
    }

protected:
    virtual void get_metas(window_meta_t* metas) {
        __super::get_metas(metas);
        metas->exstyle |= WS_EX_ACCEPTFILES;
    }

    virtual LPCTSTR get_skin_xml() const override;

    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;

    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override;

    virtual void on_final_message() override {
        delete this;
    }

private:
    void _refresh();

    void _execute(int i);
};

/*
void create_main(taoexec::model::item_db_t& db, taoexec::model::config_db_t& cfg) {
    TW* ptw = new TW(db, cfg);
    ptw->create();
    ptw->show();
}*/

} // namespace view

} // namespace taoexec
