#include <windows.h>

#include "charset.h"

static std::wstring __ae2u(UINT cp, const std::string& src) {
    if (src.size() > 0) {
        int cch = ::MultiByteToWideChar(cp, MB_PRECOMPOSED, src.c_str(), -1, nullptr, 0);
        if (cch > 0) {
            wchar_t* ws = new wchar_t[cch];
            if (::MultiByteToWideChar(cp, MB_PRECOMPOSED, src.c_str(), -1, ws, cch) > 0) {
                std::wstring r(ws);
                delete[] ws;
                return r;
            }
            else {
                delete[] ws;
            }
        }
    }
    return std::wstring();
}

static std::string __u2ae(UINT cp, const std::wstring& src) {
    if (src.size() > 0) {
        int cb = ::WideCharToMultiByte(cp, 0, src.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (cb > 0) {
            char* ms = new char[cb];
            if (::WideCharToMultiByte(cp, 0, src.c_str(), -1, ms, cb, nullptr, nullptr) > 0) {
                std::string r(ms);
                delete[] ms;
                return r;
            }
            else {
                delete[] ms;
            }
        }
    }
    return std::string();
}

namespace nbsg {
    namespace charset {
        std::wstring a2u(const std::string& src) {
            return __ae2u(CP_ACP, src);
        }

        std::string u2a(const std::wstring& src) {
            return __u2ae(CP_ACP, src);
        }

        std::wstring e2u(const std::string& src) {
            return __ae2u(CP_UTF8, src);
        }

        std::string u2e(const std::wstring& src) {
            return __u2ae(CP_UTF8, src);
        }

        std::string a2e(const std::string& src) {
            return u2e(a2u(src));
        }

        std::string e2a(const std::string& src) {
            return u2a(e2u(src));
        }

#if defined(_UNICODE) || defined(UNICODE)
        std::wstring e2t(const std::string& src) {
            return e2u(src);
        }
#else
        std::string  e2t(const std::string& src) {
            return e2a(src);
        }
#endif

    }
}

