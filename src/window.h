#pragma once

#include <climits>
#include <algorithm>
#include <windows.h>
#include <commctrl.h>

#undef min
#undef max

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

        struct layout_struct_t {
            // 当前可用的最大宽度与高度 
            int width_avail;
            int height_avail;

            // 当前的起始坐标
            int x;
            int y;

            // 本次布局所占据的宽度与高度
            int width_used;
            int height_used;
        };

        class layout_object_i {
        public:
            virtual void set_pos(layout_struct_t* ls, bool test = false) = 0;
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
        class layout_container_t;

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
            layout_container_t* _root;
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
            virtual void set_pos(layout_struct_t* ls, bool test) override {
                ls->height_used = 50;
                ls->width_used = 50;

                if(!test) {
                    ::SetWindowPos(*this, nullptr, ls->x, ls->y, 50, 50, SWP_NOZORDER);
                }
            }

        /*protected:*/
        public:
            RECT            _rect;
            layout_object_t _layout;
        };


        class layout_container_t : public child_window_t {
        public:
            virtual bool create(
                HWND parent,
                UINT id = 0,
                const std::string& text = std::string(),
                RECT rect = { 10, 10, 60, 60 }
                ) override {
                return true;
            }
        public: // interfaces
            virtual void set_pos(layout_struct_t* ls, bool test) override {
                int cw = ls->width_avail;   // client width
                int ch = ls->height_avail;  // client height

                int dw = 0; // desired width
                int dh = 0; // desired height

                int x = ls->x;  // position axis x
                int y = ls->y;  // position axis y

                int lw = 0; // current line width, max
                int lh = 0; // current line height, max

                for (int i = 0; i < _children.size(); i++) {
                    auto obj = _children[i];
                    auto& layout = obj->_layout;
                    SIZE sz = obj->set_pos({ x, y, 9999, 9999 });

                    static auto calc_relative = [](RECT* rc, const nbsg::window::layout_object_t& layout) {
                        using namespace nbsg::window;
                        auto& flags = layout.flags;
                        if (flags & layout_object_t::attr::left) {
                            rc->left += layout.left;
                            rc->right += layout.left;
                        }
                        else if (flags & layout_object_t::attr::right) {
                            rc->left -= layout.right;
                            rc->right -= layout.right;
                        }

                        if (flags & layout_object_t::attr::top) {
                            rc->top += layout.top;
                            rc->bottom += layout.top;
                        }
                        else if (flags & layout_object_t::attr::bottom) {
                            rc->top -= layout.bottom;
                            rc->bottom -= layout.bottom;
                        }
                    };

                    if (layout.display != "none") {
                        if (layout.position == "static" || layout.position == "relative") {
                            bool is_relative = layout.position == "relative";

                            if (layout.display == "inline") {
                                // if exceeds cw
                                bool new_line = false;
                                if (lw + sz.cx > cw) {
                                    new_line = true;
                                    // switch to next line
                                    x = 0;
                                    y += lh;
                                    //dh += lh;
                                    lh = 0;
                                    lw = 0;
                                }

                                RECT rc = { x, y, x + sz.cx, y + sz.cy };
                                if (is_relative)
                                    calc_relative(&rc, layout);
                                obj->set_pos(rc);

                                x += sz.cx;

                                // new desired height max
                                if (new_line) {
                                    dh += sz.cy;
                                    lh = sz.cy;
                                }
                                else {
                                    if (sz.cy > lh) {
                                        dh += sz.cy - lh;
                                        lh = sz.cy;
                                    }
                                    else {
                                        lh = sz.cy;
                                    }
                                }

                                lw += sz.cx;
                                if (lw > dw)
                                    dw = lw;
                            }
                            else if (layout.display == "block") {
                                x = 0;
                                y += lh;
                                lh = 0;

                                RECT rc = { x, y, x + /*sz.cx*/ cw, y + sz.cy };
                                if (is_relative)
                                    calc_relative(&rc, layout);
                                obj->set_pos(rc);

                                x = 0;
                                y += sz.cy;
                                lh = 0; // already added to y

                                if (sz.cx > dw) // directly influence the dw, not lw
                                    dw = sz.cx;

                                lw = 0;
                                dh += sz.cy;
                            }
                        }
                        else if (layout.position == "absolute") {
                            using namespace nbsg::window;
                            RECT rcp = { 0, 0, cw, ch };
                            RECT rc = { 0, 0, sz.cx, sz.cy };

                            const int top = layout_object_t::attr::top;
                            const int left = layout_object_t::attr::left;
                            const int right = layout_object_t::attr::right;
                            const int bottom = layout_object_t::attr::bottom;

                            if (layout.flags & top)
                                rc.top = rcp.top + layout.top;
                            if (layout.flags & left)
                                rc.left = rcp.left + layout.left;
                            if (layout.flags & right)
                                rc.right = rcp.right - layout.right;
                            if (layout.flags & bottom)
                                rc.bottom = rcp.bottom - layout.bottom;

                            int width = sz.cx, height = sz.cy;
                            if (layout.flags & layout_object_t::attr::width)
                                width = std::max((int)sz.cx, layout.width);
                            if (layout.flags & layout_object_t::attr::height)
                                height = std::max((int)sz.cy, layout.height);

                            if (layout.flags & layout_object_t::attr::min_width)
                                width = std::max(width, layout.min_width);
                            if (layout.flags & layout_object_t::attr::min_height)
                                height = std::max(height, layout.min_height);
                            if (layout.flags & layout_object_t::attr::max_width)
                                width = std::min(width, layout.max_with);
                            if (layout.flags & layout_object_t::attr::max_height)
                                height = std::min(height, layout.max_height);

                            if (layout.flags & left && !(layout.flags & right))
                                rc.right = rc.left + width;
                            if (!(layout.flags & left) && layout.flags & right)
                                rc.left = rc.right - width;
                            if (layout.flags & top && !(layout.flags & bottom))
                                rc.bottom = rc.top + height;
                            if (!(layout.flags & top) && layout.flags & bottom)
                                rc.top = rc.bottom - height;

                            obj->set_pos(rc);
                        }
                    }
                } // end for _children
                return{ dw, dh };
            }
            
        public:
            bool add(child_window_t* c) {
                return _children.add(c);
            }
        protected:
            c_ptr_array<child_window_t> _children;
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
