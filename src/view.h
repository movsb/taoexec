#pragma once

#include <taowin/core/tw_taowin.h>

#include "event.h"
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

    protected:
        virtual void get_metas(taowin::window::WindowMeta* metas) override {
            __super::get_metas(metas);
        }

        virtual LPCTSTR get_skin_xml() const override;
        virtual bool filter_message(MSG* msg) override;
        virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);

    public:
        INPUT(const config_db_t::item_t* item = nullptr, callback_t onok = nullptr)
            : _item(item)
            , _onok(onok)
        {
        }

    protected:
        virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr);

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
    virtual void get_metas(taowin::window::WindowMeta* metas) override {
        __super::get_metas(metas);
        metas->style = WS_POPUP;
        metas->exstyle = WS_EX_TOPMOST | WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_LAYERED;
    }

    virtual LPCTSTR get_skin_xml() const override;
    virtual bool filter_message(MSG* msg) override;
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam);

    void set_display(int cmd);

    virtual void on_final_message() override {
        __super::on_final_message();
        delete this;
    }

public:
    MINI(taoexec::model::item_db_t& db, taoexec::model::config_db_t& cfg);

    ~MINI() {

    }

private:
    HWND _focus;
    taoexec::model::item_db_t& _db;
    taoexec::model::config_db_t& _cfg;

private:
    struct event_exec_args : taoexec::eventx::event_args_i
    {
        std::string commander;
        std::string args;
    };

protected:
    // 命令行支持的模式：
    /*
     *  [commander] [:] [command-specs]
     **/
    void execute(std::string& __args);

    eventx::event_cookies_t _event_cookies;
    void _init_event_listeners();
};

class TW : public taowin::window_creator {
private:
    taoexec::model::item_db_t& _db;
    taoexec::model::config_db_t& _cfg;
    std::vector<taoexec::model::item_t*> _items;

	class ItemDataSource : public taowin::ListViewControl::IDataSource
	{
	private:
		const std::vector<taoexec::model::item_t*>* _items;

	public:
		ItemDataSource()
			: _items(nullptr)
		{

		}

		void SetItems(const std::vector<taoexec::model::item_t*>* items)
		{
			_items = items;
		}

	public:
		virtual size_t size() const override
		{
			return _items != nullptr ? _items->size() : 0;
		}

		virtual LPCTSTR get(int row, int column) const override
		{
			if (_items == nullptr) return _T("");

			auto rit = (*_items)[row];
			auto value = _T("");

			switch (column)
			{
			case 0: value = rit->id.c_str();                 break;
			case 1: value = rit->index.c_str();              break;
			case 2: value = rit->group.c_str();              break;
			case 3: value = rit->comment.c_str();            break;
			case 4: value = rit->paths.c_str();              break;
			case 5: value = rit->params.c_str();			 break;
			case 6: value = rit->work_dir.c_str();			 break;
			case 7: value = rit->env.c_str();                break;
			case 8: value = (rit->show ? _T("1") : _T("0")); break;
			}

			return value;
		}
	};

public:
    TW(taoexec::model::item_db_t& db, taoexec::model::config_db_t& cfg) 
        : _db(db)
        , _cfg(cfg)
    {
		_source.SetItems(&_items);
    }

protected:
    virtual void get_metas(WindowMeta* metas) {
        __super::get_metas(metas);
        metas->exstyle |= WS_EX_ACCEPTFILES;
    }

    virtual LPCTSTR get_skin_xml() const override;
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override;
	virtual taowin::syscontrol* filter_control(HWND hwnd) override;
    virtual void on_final_message() override {
        delete this;
    }

private:
    eventx::event_cookies_t _event_cookies;
    void _init_event_listeners();

    void _refresh();
    void _execute(int i);

private:
	taowin::ListViewControl* _list;
	ItemDataSource _source;
};

} // namespace view

} // namespace taoexec
