#pragma once

#include <string>
#include <vector>
#include <functional>

#include <sqlite3/sqlite3.h>

namespace taoexec {
    namespace model {
        struct item_t {
            std::string     id;
            std::string     index;
            std::string     group;
            std::string     comment;
            std::string     path;
            std::string     params;
            std::string     work_dir;
            std::string     env;
            bool            show;

            item_t() {
                id = -1;
            }
        };

        class db_t {
        public:
            db_t ()
                : _db(nullptr)
            {}

            int open(const std::string& file);
            int close();

        public:
            bool    has(int i);
            int     insert(const item_t* item);
            int     remove(const std::string& where);
            bool    remove(int id);
            int     query(const std::string& pattern, std::vector<item_t*>* items);
            int     query(const std::string& pattern, std::function<bool(item_t& item)> callback);
            item_t* query(int id);
            bool    modify(const item_t* item);

        private:
            int _create_tables();

        private:
            sqlite3*        _db;
        };
    }
}
