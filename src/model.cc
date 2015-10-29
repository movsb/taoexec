#include "model.h"

namespace nbsg {
    namespace model {

        int db_t::open(const std::string& file) {
            int rv = ::sqlite3_open(file.c_str(), &_db);
            if(rv != SQLITE_OK) {
                ::sqlite3_close(_db);
                _db = nullptr;
                return -1;
            }

            _create_tables();

            return 0;
        }

        int db_t::close() {
            if(::sqlite3_close(_db) == SQLITE_OK) {
                _db = nullptr;
                return 0;
            }

            return -1;
        }

        int db_t::_create_tables() {
            char* err;
            const char* sql = "CREATE TABLE IF NOT EXISTS items ("
                "id INTEGER PRIMARY KEY,"
                "index_ TEXT,"
                "group_ TEXT,"
                "comment TEXT,"
                "path TEXT,"
                "params TEXT,"
                "work_dir TEXT,"
                "env TEXT,"
                "show INTEGER DEFAULT 1"
                ")";

            if(::sqlite3_exec(_db, sql, nullptr, nullptr, &err) == SQLITE_OK) {
                return 0;
            }

            std::string strerr(err);
            ::sqlite3_free(err);

            return -1;
        }

        // fails on return value <= 0
        int db_t::insert(const item_t* item) {
            if(!item || std::atoi(item->id.c_str()) != -1) {
                return -1;
            }

            const char* sql = "INSERT INTO items (index_,group_,comment,path,params,work_dir,env,show)"
                " VALUES (?,?,?,?,?,?,?,?);";

            sqlite3_stmt* stmt;
            if(::sqlite3_prepare(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
                return -1;
            }

            bool next = true;
            next = next && ::sqlite3_bind_text(stmt, 1, item->index.c_str(), item->index.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 2, item->group.c_str(), item->group.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 3, item->comment.c_str(), item->comment.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 4, item->path.c_str(), item->path.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 5, item->params.c_str(), item->params.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 6, item->work_dir.c_str(), item->work_dir.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 7, item->env.c_str(), item->env.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_int (stmt, 8, item->show ? 1 : 0) == SQLITE_OK;

            if(next == false) {
                ::sqlite3_finalize(stmt);
                return -1;
            }

            if(::sqlite3_step(stmt) != SQLITE_DONE) {
                ::sqlite3_finalize(stmt);
                return -1;
            }

            ::sqlite3_finalize(stmt);

            return (int)::sqlite3_last_insert_rowid(_db);
        }

        int db_t::remove(const std::string& where) {
            return -1;
        }

        bool db_t::remove(int id) {
            std::string sql("DELETE FROM items WHERE id=");
            sql += std::to_string(id) + ";"; // + " LIMIT 1; "; sqlite3 does not support limit 1

            char* err;
            bool ok = ::sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK;
            if(!ok) ::sqlite3_free(err);
            return ok;
        }

        bool db_t::has(int i) {
            std::string sql("SELECT id FROM items WHERE id=");
            sql += std::to_string(i) + " LIMIT 1;";
            sqlite3_stmt* stmt;
            if(::sqlite3_prepare(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                return false;
            }

            bool has_ = ::sqlite3_step(stmt) == SQLITE_ROW;
            ::sqlite3_finalize(stmt);

            return has_;
        }

        // TODO ºÏ²¢
        int db_t::query(const std::string& pattern, std::vector<item_t*>* items) {
            const char* sql = "SELECT * FROM items WHERE index_ like ?;";
            sqlite3_stmt* stmt;
            const char* err = nullptr;
            if(::sqlite3_prepare(_db, sql, -1, &stmt, &err) != SQLITE_OK) {
                return -1;
            }

            std::string likestr = pattern + "%%";
            if(::sqlite3_bind_text(stmt, 1, likestr.c_str(), likestr.size(), nullptr) != SQLITE_OK) {
                ::sqlite3_finalize(stmt);
                return -1;
            }

            items->clear();

            int sr, n = 0;
            while((sr = ::sqlite3_step(stmt)) == SQLITE_ROW) {
                item_t* pi = new item_t;
                item_t& i = *pi;

                i.id = std::to_string(::sqlite3_column_int(stmt, 0));
                i.index = (char*)::sqlite3_column_text(stmt, 1);
                i.group = (char*)::sqlite3_column_text(stmt, 2);
                i.comment = (char*)::sqlite3_column_text(stmt, 3);
                i.path = (char*)::sqlite3_column_text(stmt, 4);
                i.params = (char*)::sqlite3_column_text(stmt, 5);
                i.work_dir = (char*)::sqlite3_column_text(stmt, 6);
                i.env = (char*)::sqlite3_column_text(stmt, 7);
                i.show = !!::sqlite3_column_int(stmt, 8);

                items->push_back(pi);

                n++;
            }

            if (sr != SQLITE_DONE) {    // TODO remove all
                return -1;
            }

            return n;
        }

        int db_t::query(const std::string& pattern, std::function<bool(item_t& item)> callback) {
            return -1;
            /*
            const char* sql = "SELECT * FROM items;";
            sqlite3_stmt* stmt;
            if (::sqlite3_prepare(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
                return -1;
            }

            int sr, n = 0;
            while ((sr = ::sqlite3_step(stmt)) == SQLITE_ROW) {
                item_t i;
                
                i.id         = ::sqlite3_column_int(stmt, 0);
                i.index      = (char*)::sqlite3_column_text(stmt, 1);
                i.group      = (char*)::sqlite3_column_text(stmt, 2);
                i.comment    = (char*)::sqlite3_column_text(stmt, 3);
                i.path       = (char*)::sqlite3_column_text(stmt, 4);
                i.params     = (char*)::sqlite3_column_text(stmt, 5);
                i.work_dir   = (char*)::sqlite3_column_text(stmt, 6);
                i.env        = (char*)::sqlite3_column_text(stmt, 7);
                i.show = !!::sqlite3_column_int(stmt, 8);

                callback(i);

                n++;
            }

            if (sr != SQLITE_DONE) {    // TODO remove all
                return -1;
            }

            return n;
            */
        }

        item_t* db_t::query(int id) {
            auto sql = "SELECT * FROM items WHERE id=" + std::to_string(id) + " LIMIT 1;";
            sqlite3_stmt* stmt;
            if(::sqlite3_prepare(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                return nullptr;
            }

            int sr = ::sqlite3_step(stmt);
            if(sr != SQLITE_ROW)
                return nullptr;

            auto pi = new item_t;
            auto& i = *pi;

            i.id = ::sqlite3_column_int(stmt, 0);
            i.index = (char*)::sqlite3_column_text(stmt, 1);
            i.group = (char*)::sqlite3_column_text(stmt, 2);
            i.comment = (char*)::sqlite3_column_text(stmt, 3);
            i.path = (char*)::sqlite3_column_text(stmt, 4);
            i.params = (char*)::sqlite3_column_text(stmt, 5);
            i.work_dir = (char*)::sqlite3_column_text(stmt, 6);
            i.env = (char*)::sqlite3_column_text(stmt, 7);
            i.show = !!::sqlite3_column_int(stmt, 8);

            return pi;
        }

        bool db_t::modify(const item_t* item) {
            const char* sql = "UPDATE items SET index_=?,group_=?,comment=?,path=?,params=?,work_dir=?,env=?,show=? WHERE id=?;";

            sqlite3_stmt* stmt;
            if(::sqlite3_prepare(_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
                return false;
            }

            bool next = true;
            next = next && ::sqlite3_bind_text(stmt, 1, item->index.c_str(), item->index.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 2, item->group.c_str(), item->group.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 3, item->comment.c_str(), item->comment.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 4, item->path.c_str(), item->path.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 5, item->params.c_str(), item->params.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 6, item->work_dir.c_str(), item->work_dir.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_text(stmt, 7, item->env.c_str(), item->env.size(), nullptr) == SQLITE_OK;
            next = next && ::sqlite3_bind_int(stmt, 8, item->show ? 1 : 0) == SQLITE_OK;
            next = next && ::sqlite3_bind_int(stmt, 9, std::atoi(item->id.c_str())) == SQLITE_OK;

            if(next == false) {
                ::sqlite3_finalize(stmt);
                return false;
            }

            if(::sqlite3_step(stmt) != SQLITE_DONE) {
                ::sqlite3_finalize(stmt);
                return false;
            }

            ::sqlite3_finalize(stmt);

            return true;
        }

    }
}
