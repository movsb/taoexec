#pragma once

#include <queue>

#include <windows.h>

namespace taoweb {
    class locker {
    public:
        locker() {
            ::InitializeCriticalSection(&_cs);
        }

        virtual ~locker() {
            ::DeleteCriticalSection(&_cs);
        }

        void lock() {
            return ::EnterCriticalSection(&_cs);
        }

        void unlock() {
            return ::LeaveCriticalSection(&_cs);
        }

        bool try_lock() {
            return !!::TryEnterCriticalSection(&_cs);
        }

    protected:
        CRITICAL_SECTION _cs;
    };

    template<typename T>
    class lock_queue {
    public:
        lock_queue() {
            _h_event = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
        }

        ~lock_queue() {
            ::CloseHandle(_h_event);
            _h_event = nullptr;
        }

    public:
        void push(T& client) {
            _queue.push(client);
            ::SetEvent(_h_event);
            return;
        }

        T pop() {
            T p;

            for (;;) {
                if (!size()) {
                    ::WaitForSingleObject(_h_event, INFINITE);
                }

                if (!_lock.try_lock())
                    continue;

                if (!size()) {
                    _lock.unlock();
                    continue;
                }

                p = _queue.front();
                _queue.pop();

                _lock.unlock();

                break;
            }

            return p;
        }

        int size() {
            return _queue.size();
        }

    protected:
        locker  _lock;
        std::queue<T>   _queue;
        HANDLE          _h_event;
    };

    class lock_count {
    public:
        lock_count()
            : _count(0)
        {}

        long size() {
            return _count;
        }

        void inc() {
            ::InterlockedIncrement(&_count);
        }

        void dec() {
            ::InterlockedDecrement(&_count);
        }
    private:
        long _count;
    };

}
