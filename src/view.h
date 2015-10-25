#pragma once

#include <taoweb/taoweb.hpp>

#include <regex>

#include "charset.h"
#include "view.h"
#include "core.h"
#include <shellapi.h>

namespace nbsg {
    namespace view {
        class handler_t : public taoweb::http::http_handler_t {
        public:
            handler_t(taoweb::client_t& client, taoweb::http::http_header_t& header)
                : http_handler_t(client, header) {}

            virtual ~handler_t() {

            }

            void set_db(nbsg::model::db_t* db) {
                _pdb = db;
            }

        private:
            nbsg::model::db_t* _pdb;

        public:
            virtual void handle() {
                auto& uri = _header._uri_decoded;
                if(uri == "/" || uri == "/index")
                    return _handle_list_all();
                else if(uri == "/list")
                    return _handle_list_all();
                else if(uri == "/add")
                    return _handle_add();
                else if(uri == "/delete")
                    return _handle_delete();
                else if(uri == "/modify") {
                    return _handle_modify();
                }
                else if(uri == "/run")
                    return _handle_run();
                else {
                    taoweb::http::static_http_handler_t handler(_client, _header);
                    handler.set_root(taoweb::file_system::exe_dir() + "/web");
                    handler.handle();
                }
            }

        protected:
            void _send(const std::string& s) {
                ::send(_client.fd, s.c_str(), s.size(), 0);
            }

            void _send(const char* s) {
                ::send(_client.fd, s, ::strlen(s), 0);
            }

            void _send_json(const std::string& json) {
                std::stringstream ss;
                ss << "HTTP/1.1 200 OK\r\n"
                    << "Date: " + taoweb::http::gmtime() + "\r\n"
                    << "Content-Type: text/json\r\n"
                    << "Content-Length: " << json.size() << "\r\n"
                    << "\r\n";

                _send(ss.str());
                _send(json);
            }

            void close() {
                ::closesocket(_client.fd);
            }

        protected:
            void _handle_list_all() {
                _send("HTTP/1.1 200 OK\r\n");
                _send("Date: " + taoweb::http::gmtime() + "\r\n");
                _send("\r\n");
                _send(R"(<!doctype html>
<html>
<head>
    <meta charset="UTF-8" />
    <title>nbsg::list_all</title>
    <link rel="stylesheet" type="text/css" href="/style.css" />
    <script src="/jquery.js"></script>
</head>
<body>
<div id="dlg-add" style="position: fixed; left: 0px; top: 0px; width: 100%; height: 100%;">
    <div style="width: 500px; height: 400px; border: 1px solid blue; background-color: white; position: relative; margin: auto; top: 50%; transform: translateY(-50%);">
        <form>
            <div class="titlebar" style="text-align: center; margin-bottom: 1em;">
                <span style="border-bottom: 1px solid blue; display: block; width: 95%; margin: 0px auto; font-size: 2em;">New</span>
            </div>
            <div class="content" style="margin-top: 1em; padding: 1em;">
                <div>
                    <label>id</label><input type="text" name="id"/>
                </div>
                <div>
                    <label>index</label><input type="text" name="index"/>
                </div>
                <div>
                    <label>group</label><input type="text" name="group"/>
                </div>
                <div>
                    <label>comment</label><input type="text" name="comment"/>
                </div>
                <div>
                    <label>path</label><input type="text" name="path"/>
                </div>
                <div>
                    <label>params</label><input type="text" name="params"/>
                </div>
                <div>
                    <label>work_dir</label><input type="text" name="work_dir"/>
                </div>
                <div>
                    <label>env</label><input type="text" name="env"/>
                </div>
                <div>
                    <label>visibility</label><input type="text" name="visibility"/>
                </div>
                <div style="display: none;">
                    <input type="hidden" name="do" value="add" />
                </div>
            </div>
            <div class="footer">
                <input type="submit" id="dlg-add-submit" value="save" />
            </div>
        </form>
    </div>
</div>
<div>
    <button id="add-item">add new</button>
</div>
<table id="items">
    <thead>
        <tr>
            <th>id</th>
            <th>index</th>
            <th>group</th>
            <th>comment</th>
            <th>path</th>
            <th>path(expanded)</th>
            <th>params</th>
            <th>work_dir</th>
            <th>env</th>
            <th>visibility</th>
            <th>edit</th>
            <th>delete</th>
            <th>run</th>
        </tr>
    </thead>
    <tbody>
)");

                _pdb->query("", [this](nbsg::model::item_t& item) {
                    std::stringstream res;
                    res << "<tr>\n"
                        << "<td class='id'>"        << item.id            << "</td>\n"
                        << "<td class='index'>"     << item.index         << "</td>\n"
                        << "<td class='group'>"     << item.group         << "</td>\n"
                        << "<td class='comment'>"   << item.comment       << "</td>\n"
                        << "<td class='path'>"      << item.path          << "</td>\n"
                        << "<td class='path_expanded'>"     << nbsg::core::expand(item.path) << "</td>\n"
                        << "<td class='params'>"    << item.params        << "</td>\n"
                        << "<td class='work_dir'>"  << item.work_dir      << "</td>\n"
                        << "<td class='env'>"       << item.env           << "</td>\n"
                        << "<td class='visibility'>"        << item.visibility      << "</td>\n"
                        << "<td>" << R"(<button class='edit'>edit</button>)"   << "</td>\n"
                        << "<td>" << R"(<button class='delete'>delete</button>)"   << "</td>\n"
                        << "<td>" << R"(<button class='run'>run</button>)"    << "</td>\n"
                        ;
                    
                    _send(res.str());

                    return true;
                });

                _send("</tbody>\n</table>\n");
                _send(R"(<script src="/script.js"></script></body></html>)");
    
                ::closesocket(_client.fd);
            }

            void _handle_run() {
                _send_json(R"({"errno":"ok"})");
                close();

                auto& query = _header._query;

                auto id = std::stoi(taoweb::http::query_field(_header._queries, "id"));
                auto pi = _pdb->query(id);

                ::ShellExecute(nullptr, "open", nbsg::core::expand(pi->path).c_str(), pi->params.c_str(), pi->work_dir.c_str(), SW_SHOWNORMAL);
            }

            void _handle_add() {
                auto& q = _header._queries;
                nbsg::model::item_t item;
                item.id = std::stoi(taoweb::http::query_field(q, "id", "-1"));
                item.index = taoweb::http::query_field(q, "index", "");
                item.group = taoweb::http::query_field(q, "group", "");
                item.comment = taoweb::http::query_field(q, "comment", "");
                item.path = taoweb::http::query_field(q, "path", "");
                item.params = taoweb::http::query_field(q, "params", "");
                item.work_dir = taoweb::http::query_field(q, "work_dir", "");
                item.env = taoweb::http::query_field(q, "env", "");
                item.visibility = !!std::stoi(taoweb::http::query_field(q, "visibility", "1"));
                
                if(item.id != -1) {
                    _send_json(R"({"errno":"not ok","msg":"id must be -1"})");
                    close();
                    return;
                }

                if(_pdb->insert(&item) > 0) {
                    std::string res(R"({"errno":"ok"})");
                    _send_json(res);
                    close();
                }
                else {
                    _send_json(R"({"errno":"not ok","msg":"error insert"})");
                    close();
                }
            }

            void _handle_delete() {
                int id = std::stoi(taoweb::http::query_field(_header._queries, "id"));
                if(!_pdb->has(id)) {
                    std::string res(R"({"errno":"not ok","msg":"id does not exist."})");
                    _send_json(res);
                    close();
                }

                _send_json(_pdb->remove(id)
                    ? R"({"errno":"ok"})"
                    : R"({"errno":"not ok","msg":"unknown."})"
                    );
                close();
            }

            void _handle_modify() {
                auto& q = _header._queries;
                nbsg::model::item_t item;
                item.id = std::stoi(taoweb::http::query_field(q, "id", "-1"));
                item.index = taoweb::http::query_field(q, "index", "");
                item.group = taoweb::http::query_field(q, "group", "");
                item.comment = taoweb::http::query_field(q, "comment", "");
                item.path = taoweb::http::query_field(q, "path", "");
                item.params = taoweb::http::query_field(q, "params", "");
                item.work_dir = taoweb::http::query_field(q, "work_dir", "");
                item.env = taoweb::http::query_field(q, "env", "");
                item.visibility = !!std::stoi(taoweb::http::query_field(q, "visibility", "1"));

                if(!_pdb->has(item.id)) {
                    _send_json(R"({"errno":"not ok","msg":"item does not exist."})");
                    close();
                    return;
                }

                if(!_pdb->modify(&item)) {
                    _send_json(R"({"errno":"not ok","msg":"error."})");
                    close();
                }
                else {
                    _send_json(R"({"errno":"ok"})");
                    close();
                }
            }
        };
    }
}
