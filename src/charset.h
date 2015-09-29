#pragma once

#include <string>

/*
    A - ANSI(extended)
    U - UNICODE(16)
    E - UTF-8
    T - typedef-ed
*/

namespace nbsg {
    namespace charset {
        // from ANSI(extended) to UNICODE
        std::wstring a2u(const std::string& src);

        // from UNICODE to ANSI(extended)
        std::string  u2a(const std::wstring& src);

        // from UTF-8 to UNICODE
        std::wstring e2u(const std::string& src);

        // from UNICODE to UTF-8
        std::string  u2e(const std::wstring& src);

        // from ANSI(extended) to UTF-8
        std::string  a2e(const std::string& src);

        // from UTF-8 to ANSI(extended)
        std::string  e2a(const std::string& src);

        // from UTF-8 to ANSI or UNICODE
#if defined(_UNICODE) || defined(UNICODE)
        std::wstring e2t(const std::string& src);
#else
        std::string  e2t(const std::string& src);
#endif

    }
}
