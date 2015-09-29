#include <cstdlib>
#include <vector>

#include "window.h"

namespace nbsg {
    namespace window {

        c_ptr_array<message_filter_i> g_filters;

        // message loop
        void message_loop_t::loop() {
            while (::GetMessage(&_msg, NULL, 0, 0)) {
                if (!_filter_message()) {
                    ::TranslateMessage(&_msg);
                    ::DispatchMessage(&_msg);
                }
            }
        }

        bool message_loop_t::_filter_message() {
            HWND top_wnd = _msg.hwnd;
            while (::GetWindowLongPtr(top_wnd, GWL_STYLE) & WS_CHILD)
                top_wnd = ::GetParent(top_wnd);

            for (int i = 0; i < g_filters.size(); i++) {
                if (g_filters[i]->filter_hwnd() == top_wnd) {
                    if(g_filters[i]->filter_message(&_msg))
                        return true;
                    break;
                }
            }
            return false;
        }


        bool frame_window_t::create(const std::string& text , RECT rect) {
            ::AdjustWindowRectEx(&rect, _get_window_style(), FALSE, _get_window_ex_style());
            _hwnd = ::CreateWindowEx(_get_window_ex_style(), "tao_frame_window", text.c_str(), _get_window_style(),
                rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                nullptr, nullptr, nullptr, this);

            if (!_hwnd) {
                return false;
            }

            g_filters.add(this);

            return true;
        }

        bool frame_window_t::_global_init() {
            static bool inited = false;
            if (!inited) {
                WNDCLASSEX wc = { 0 };
                wc.cbSize = sizeof(wc);
                wc.cbWndExtra = sizeof(void*);
                wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
                wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
                wc.hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
                wc.hIconSm = wc.hIcon;
                wc.hInstance = ::GetModuleHandle(nullptr);
                wc.lpfnWndProc = &frame_window_t::__wndproc;
                wc.lpszClassName = "tao_frame_window";
                wc.style = CS_HREDRAW | CS_VREDRAW;
                
                if (RegisterClassEx(&wc)) {
                    inited = true;
                }
            }

            return inited;
        }

        frame_window_t::frame_window_t() {
            frame_window_t::_global_init();
        }

        LRESULT frame_window_t::wndproc(UINT msg, WPARAM wp, LPARAM lp) {
            switch (msg) {
            case WM_CLOSE:
                ::DestroyWindow(_hwnd);
                return 0;
            case WM_PRINTCLIENT:
            {
                HDC hdc = HDC(wp);
                DWORD opt = DWORD(lp);
                opt = opt;
            }
            }

            return ::DefWindowProc(_hwnd, msg, wp, lp);
        }

        LRESULT CALLBACK frame_window_t::__wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
            auto that = reinterpret_cast<frame_window_t*>(::GetWindowLongPtr(hwnd, 0));
            if (that != nullptr) { return that->wndproc(msg, wp, lp); }

            if (msg == WM_CREATE) {
                LPCREATESTRUCT cs = reinterpret_cast<LPCREATESTRUCT>(lp);
                auto that = reinterpret_cast<frame_window_t*>(cs->lpCreateParams);
                that->_hwnd = hwnd;
                ::SetWindowLongPtr(hwnd, 0, LONG(that));
                return that->wndproc(msg, wp, lp);
            }

            return ::DefWindowProc(hwnd, msg, wp, lp);
        }

        bool frame_window_t::show(bool show_ /*= true*/, bool focus /*= true*/) {
            int state = SW_SHOW;
            if (show_) {
                if (focus) {
                    state = SW_SHOWNORMAL;
                }
                else {
                    state = SW_SHOW;
                }
            }
            else {
                state = SW_HIDE;
            }

            ::ShowWindow(_hwnd, state);
            return !!::IsWindowVisible(_hwnd);
        }

        bool frame_window_t::filter_message(MSG* msg) {
            if (msg->message == WM_KEYDOWN && msg->wParam == VK_ESCAPE) {
                ::SendMessage(_hwnd, WM_CLOSE, 0, 0);
                return true;
            }
            return false;
        }

        HWND frame_window_t::filter_hwnd() {
            return _hwnd;
        }


        // list view
        bool listview_child_t::create(HWND parent, UINT id, const std::string& text, RECT rect)
        {
            _hwnd = ::CreateWindowEx(_get_window_ex_style(), WC_LISTVIEW, text.c_str(), _get_window_style(),
                rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                parent, HMENU(id), nullptr, 0);

            if (_hwnd) {
                set_extended_style(0
                    | LVS_EX_FULLROWSELECT
                    | LVS_EX_GRIDLINES
                    );
            }
            return !!_hwnd;
        }
        
        // list view methods
        int listview_child_t::insert_column(int index, const LVCOLUMN* c) {
            return ListView_InsertColumn(_hwnd, index, c);
        }

    }
}
