#pragma once

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <tuple>

#include "types.hpp"

#include <windows.h>

namespace taoexec {
namespace eventx {

    // 事件参数原型
    struct event_args_i
    {
        virtual ~event_args_i() {

        }
    };

    // 常用事件
    struct event_msgbox_args : event_args_i
    {
        std::string title;
        std::string content;
        unsigned int type;
        void* hwnd;

        event_msgbox_args(conststring& content_, conststring& title_ = "", unsigned int type_ = MB_OK, void* hwnd_ = (void*)::GetActiveWindow()) {
            title = title_;
            content = content_;
            type = type_;
            hwnd = hwnd_;
        }
    };

    // 事件处理器
    class event_handler
    {
    public:
        event_handler(std::function<bool(event_args_i* args)> fn)
            : _fn(fn)
        {
        }

        bool operator()(event_args_i* args) {
            return _fn(args);
        }

    protected:
        std::function<bool(event_args_i* args)> _fn;
    };

    // 事件处理器容器
    class event_handler_container_t
    {
    protected:
        std::vector<event_handler*> _handlers;

    public:
        event_handler_container_t() {}
        ~event_handler_container_t()
        try {
            for_each([](int i, event_handler* p) {
                delete p;
                return false;
            });
        }
        catch(...) {

        }

    public:
        bool call(event_args_i* args) {
            bool ok = true;
            for_each([&](int i, event_handler* p) {
                auto r = (*p)(args);
                if(!r) ok = false;
                return r;
            });
            return ok;
        }

        void add(event_handler* p) {
            auto r = has(p);
            if(!std::get<0>(r)) {
                _handlers.push_back(p);
            }
        }

        void del(event_handler* p) {
            auto r = has(p);
            if(std::get<0>(r)) {
                _handlers.erase(_handlers.cbegin() + std::get<1>(r));
            }
        }

        std::tuple<bool, int> has(event_handler* p) {
            bool f = false;
            int i;

            for_each([&](int i, event_handler* e) {
                if(e == p) {
                    f = true;
                    return true;
                }
                else {
                    return false;
                }
            });

            return std::make_tuple(f, i);
        }

        int size() const {
            return _handlers.size();
        }

        void for_each(std::function<bool(int i, event_handler*)> fx) const {
            int i = 0;
            for(auto& h : _handlers) {
                if(fx(i++, h))
                    break;
            }
        }

    };

    // 
    typedef std::map<const char*, event_handler*> event_cookies_t;

    // 事件管理器
    class event_manager_t
    {
    public:

    public:
        event_handler* attach(const std::string& name, std::function<bool(event_args_i*)> p) {
            auto rp = new event_handler(p);
            _events[name].add(rp);
            return rp;
        }

        void detach(const std::string& name, event_handler* p) {
            auto it = _events.find(name);
            if(it != _events.cend())
                it->second.del(p);
        }

        bool trigger(const std::string& name, event_args_i* args = nullptr) {
            auto& container = _events[name];

#if !defined(__PR__)
            if (!container.size() && name != "msgbox") {
                auto msg = new event_msgbox_args("事件 " + name + " 没有处理器。");
                trigger("msgbox", msg);
            }
#endif

            auto r = container.call(args);
            delete args;
            return r;
        }

    protected:
        std::map<std::string, event_handler_container_t>    _events;
    };


} // namespace eventx
} // namespace taoexec

extern taoexec::eventx::event_manager_t* _evtmgr;
