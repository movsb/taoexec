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

        class child_window_t;

        class frame_window_t : public window_base_t, message_filter_i {
        public:
            frame_window_t();

            virtual bool create(const std::string& text = std::string(), RECT rect = {100, 100, 300, 300});

            bool show(bool show_ = true, bool focus = true);

        protected:
            virtual LRESULT wndproc(UINT msg, WPARAM wp, LPARAM lp);
            static LRESULT CALLBACK __wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

            virtual HWND filter_hwnd() override;
            virtual bool filter_message(MSG* msg) override;

        protected:
            static bool _global_init();

        protected: // for base
            virtual DWORD _get_window_style() {
                return WS_OVERLAPPEDWINDOW;
            }

            virtual DWORD _get_window_ex_style() {
                return 0;
            }

        protected:
            c_ptr_array<child_window_t> _children;
        };

        class child_window_t : public window_base_t {
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
