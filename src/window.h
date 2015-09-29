#pragma once

#include <climits>

#include <windows.h>
#include <commctrl.h>

namespace nbsg {
    namespace window {
        template<class T>
        class c_ptr_array
        {
        public:
            c_ptr_array()
                : m_pp(0)
                , m_cnt(0)
                , m_allocated(0) {

            }
            virtual ~c_ptr_array() {
                empty();
            }

        public:
            void empty() {
                if (m_pp) {
                    ::free(m_pp);
                }
                m_pp = 0;
                m_cnt = 0;
                m_allocated = 0;
            }

            int find(T* pv) const {
                for (int i = 0; i < m_cnt; i++) {
                    if (m_pp[i] == pv) {
                        return i;
                    }
                }
                return -1;
            }

            bool add(T* pv) {
                if (++m_cnt > m_allocated) {
                    int n = m_allocated * 2;
                    if (!n) n = 1;

                    T** pp = static_cast<T**>(::realloc(m_pp, n * sizeof(void*)));
                    if (pp) {
                        m_allocated = n;
                        m_pp = pp;
                    }
                    else {
                        --m_cnt;
                        return false;
                    }
                }
                m_pp[m_cnt - 1] = pv;
                return true;
            }

            bool remove(T* pv) {
                int i = find(pv);
                if (i == -1) {
                    assert(0);
                    return false;
                }
                else {
                    --m_cnt;
                    ::memmove(m_pp + i, m_pp + i + 1, (m_cnt - i)*sizeof(void*));
                    return true;
                }
            }

            int size() const {
                return m_cnt;
            }

            T* getat(int i) const {
                return m_pp[i];
            }

            T* operator[](int i) const {
                return getat(i);
            }

        protected:
            T** m_pp;
            int m_cnt;
            int m_allocated;
        };

        class message_filter_i {
        public:
            virtual HWND filter_hwnd() = 0;
            virtual bool filter_message(MSG* msg) = 0;
        };

        class message_loop_t {
        public:
            void loop();

        private:
            bool _filter_message();

        private:
            MSG                             _msg;
        };

        class window_base_t {
        protected:
            window_base_t() {}

        public:
            operator HWND() {
                return _hwnd;
            }

        public:
            LRESULT send_message(UINT msg, WPARAM wp = 0, LPARAM lp = 0) {
                return ::SendMessage(_hwnd, msg, wp, lp);
            }

        protected:
            virtual DWORD _get_window_style() = 0;
            virtual DWORD _get_window_ex_style() = 0;

        protected:
            HWND        _hwnd;
            UINT        _id;
            WNDPROC     _wndproc;
        };

        class layout_object_i {
        public:
            virtual void set_pos(const RECT& rect) = 0;
            virtual SIZE calc_size() = 0;
        };

        class layout_object_t {
        public:
            class attr {
            public:
                enum _has_attr {};
                // for single
                static const _has_attr left             = (_has_attr)0x00000001;
                static const _has_attr top              = (_has_attr)0x00000002;
                static const _has_attr right            = (_has_attr)0x00000004;
                static const _has_attr bottom           = (_has_attr)0x00000008;
                static const _has_attr width            = (_has_attr)0x00000010;
                static const _has_attr height           = (_has_attr)0x00000020;
                static const _has_attr max_width        = (_has_attr)0x00000040;
                static const _has_attr max_height       = (_has_attr)0x00000080;
                static const _has_attr min_width        = (_has_attr)0x00000100;
                static const _has_attr min_height       = (_has_attr)0x00000200;

                /*
                static const _has_attr display          = (_has_attr)0x00001000;
                static const _has_attr visibility       = (_has_attr)0x00002000;
                static const _has_attr position         = (_has_attr)0x00004000;

                // for position
                static const _has_attr static_          = (_has_attr)0x80000000;
                static const _has_attr absolute         = (_has_attr)0x80000001;
                static const _has_attr fixed            = (_has_attr)0x80000002;
                static const _has_attr relative         = (_has_attr)0x80000003;


                // for display
                static const _has_attr none             = (_has_attr)0x80000100;
                static const _has_attr inline_          = (_has_attr)0x80000101;
                static const _has_attr inline_block     = (_has_attr)-102;
                static const _has_attr block            = (_has_attr)-103;

                // for visibility, 使用最低位
                static const _has_attr visibility_mask  = (_has_attr)0x00000001;
                static const _has_attr visible          = (_has_attr)0x00000000;
                static const _has_attr hidden           = (_has_attr)0x00000001;
                */
            };

            layout_object_t()
                : flags(0)
                , position("static")
                , visibility("visible")
                , display("inline")
            {}

        public:
            unsigned int    flags;
            int             left, top, right, bottom;
            int             width, height;
            int             max_with, max_height, min_width, min_height;
            //bool            display, visibility;
            //int             position;
            std::string     display;
            std::string     visibility;
            std::string     position;
        };

        class child_window_t;

        class frame_window_t : public window_base_t, message_filter_i {
        public:
            frame_window_t();

            virtual bool create(const std::string& text = std::string(), RECT rect = {100, 100, 300, 300});

            bool show(bool show_ = true, bool focus = true);
            void show_scrollbar(int h, int v);

        protected:
            virtual LRESULT wndproc(UINT msg, WPARAM wp, LPARAM lp);
            static LRESULT CALLBACK __wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

            virtual HWND filter_hwnd() override;
            virtual bool filter_message(MSG* msg) override;

        protected:
            static bool _global_init();

        protected: // for base
            virtual DWORD _get_window_style() {
                return WS_OVERLAPPEDWINDOW
                    | WS_HSCROLL
                    | WS_VSCROLL
                    ;
            }

            virtual DWORD _get_window_ex_style() {
                return 0;
            }

        protected:
            c_ptr_array<child_window_t> _children;
        };

        class child_window_t : public window_base_t, public layout_object_i {
        public:
            virtual bool create(
                HWND parent,
                UINT id = 0,
                const std::string& text = std::string(),
                RECT rect = {10, 10, 60, 60}
            ) = 0;

        protected:
            virtual DWORD _get_window_style() {
                return WS_CHILD | WS_VISIBLE;
            }

            virtual DWORD _get_window_ex_style() {
                return 0;
            }

        public: // layout_object_i interfaces
            virtual void set_pos(const RECT& rect) override {
                if(::EqualRect(&_rect, &rect)) return;
                ::SetWindowPos(_hwnd, 0,
                    rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                    SWP_NOZORDER);
                _rect = rect;
            }

            virtual SIZE calc_size() override {
                RECT rc;
                ::GetWindowRect(_hwnd, &rc);
                return {rc.right - rc.left, rc.bottom - rc.top};
            }

        /*protected:*/
        public:
            RECT            _rect;
            layout_object_t _layout;
        };

        class button_child_t : public child_window_t {
        public:
            virtual bool create(
                HWND parent,
                UINT id = 0,
                const std::string& text = std::string(),
                RECT rect = {10, 10, 210, 60}
            ) override;

        };

        class listview_child_t : public child_window_t {
        public:
            virtual bool create(
                HWND parent,
                UINT id = 0,
                const std::string& text = std::string(),
                RECT rect = {10, 10, 60, 60}
            ) override;


        public: // methods
            int insert_column(int index, const LVCOLUMN* c);

            template<typename... Args>
            int insert_item(Args... args) {
                int size = sizeof...(args);
                if (size == 0) return -1;

                std::vector<std::string> a{ args... };

                LVITEM lvi;
                lvi.mask = LVIF_TEXT;
                lvi.iItem = INT_MAX;
                lvi.iSubItem = 0;
                lvi.pszText = const_cast<LPSTR>(a[0].c_str());
                if ((lvi.iItem = ListView_InsertItem(_hwnd, &lvi)) != -1 && size > 1) {
                    for (int i = 0; i < size - 1; i++) {
                        lvi.iSubItem = i + 1;
                        lvi.pszText = (LPSTR)a[i+1].c_str();
                        ListView_SetItem(_hwnd, &lvi);
                    }
                }

                return lvi.iItem;
            }

            void set_extended_style(DWORD style) {
                ListView_SetExtendedListViewStyle(_hwnd, style);
            }

        protected:
            virtual DWORD _get_window_style() override {
                return __super::_get_window_style()
                    | LVS_SHOWSELALWAYS
                    | LVS_SINGLESEL
                    | LVS_REPORT
                    ;
            }

            // list view extended styles isn't set here
            virtual DWORD _get_window_ex_style() override {
                return __super::_get_window_ex_style()
                    | WS_EX_CLIENTEDGE
                    ;
            }
        };
    }
}
