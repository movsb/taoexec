#pragma once

#include <regex>
#include <cctype>
#include <map>
#include <vector>
#include <functional>
#include <string>
#include <sstream>
#include <memory>

#include "types.hpp"
#include "charset.h"
#include "model.h"
#include "script.h"

#include <windows.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <assert.h>

namespace taoexec {
    namespace exec {
        class command_executor_i
        {
        public:
            virtual const std::string get_name() const = 0;
            virtual bool execute(const std::string& args) = 0;
        };

        /*
        class registry_executor
        {
        private:
            std::map<std::string, std::string, __string_nocase_compare> _commands;
        public:
            registry_executor();
            bool execute(const std::string& all, const std::string& scheme, const std::string& args);

        private:
            bool _execute_command(const std::string& cmd, const std::string& all);
        };
        */

        class executor_main : public command_executor_i
        {
        private:
            std::map<std::string, std::function<void()>> _cmds;

        public:
            executor_main();

            const std::string get_name() const override {
                return "__main__";
            }

            bool execute(const std::string& args) override;
        };

        class executor_indexer : public command_executor_i
        {
        private:
            taoexec::model::item_db_t* _itemdb;

        public:
            executor_indexer(taoexec::model::item_db_t* itemdb)
                : _itemdb(itemdb) {}

            const std::string get_name() const override {
                return "__indexer__";
            }

            bool execute(const std::string& args) override;
        };
        
        class executor_qq : public command_executor_i
        {
        private:
            taoexec::model::config_db_t* _cfg;
            std::string _uin;
            std::string _path;
            std::map<std::string, std::string, __string_nocase_compare> _users;

        public:
            executor_qq(taoexec::model::config_db_t* cfg);

            const std::string get_name() const override {
                return "qq";
            }

            bool execute(const std::string& args) override;
        };

        class executor_fs : public command_executor_i
        {
        private:
            class env_var_t
            {
            public:
                env_var_t() { }
                void set(const std::string& envstr);
                void patch(const std::string& envstr);
                void patch_current();
                std::string serialize() const;
                const taoexec::strstrimap& get_vars() const {
                    return _vars;
                }

            protected:
                std::vector<std::string>    _nameless;
                taoexec::strstrimap         _vars;
            };

        protected:
            typedef std::vector<std::string>                        func_args;
            typedef std::function<std::string(func_args& args)>     func_proto;
            std::map<std::string, func_proto>                       _functions;
            std::map<std::string, std::string>               		_variables;
            strstrimap                                              _exec_strs;

        protected:
            model::config_db_t&     _cfgdb;

        public:
            executor_fs(model::config_db_t& cfgdb);

            const std::string get_name() const override {
                return "fs";
            }

            bool execute(const std::string& args) override;
            bool execute(conststring& path, conststring& args, conststring& wd="", conststring& env="");

        private:
            void _expand_exec(conststring& exec_str, conststring& path, conststring& rest, std::string* __cmdline);

            // 取得目标文件（可执行与不可执行），使用变量展开与函数调用技术
            // 支持的替换：$foo() - 函数调用，${variable} - 变量展开，%var% - 环境变量展开
            void _expand_path(const std::string& before,  std::string* after);

            void _initialize_event_listners();
            void _initialize_globals();
            void _add_user_variables(const env_var_t& env_var);
            std::string _expand_variable(const std::string& var);
            std::string _expand_function(const std::string& fn, func_args& args);

            std::string _which(const std::string& cmd, const std::string& env);

            std::string get_executor(const std::string& ext);
        };

        class executor_shell : public command_executor_i
        {
        public:
            executor_shell()
            {

            }
            const std::string get_name() const override {
                return "shell";
            }

            bool execute(const std::string& args) override;
        };

        class executor_manager_t 
        {
        public:
            executor_manager_t()
            try {

            }
            catch(...) {
                throw;
            }

            ~executor_manager_t()
            try {
                _uninit_commanders();
            }
            catch(...) {
                throw;
            }

        public:
            void
                init();
            void
                add(command_executor_i* p);
            command_executor_i*
                get(const std::string& name);
            bool
                exec(const std::string& args);

        protected:
            void _init_commanders();
            void _uninit_commanders();

            void _init_event_listners();
        public:
            taoexec::model::item_db_t*      _itemdb;
            taoexec::model::config_db_t*    _cfgdb;
        protected:
            std::map<std::string, command_executor_i*, taoexec::__string_nocase_compare> _command_executors;
        };

        // ------------------------------------------------------------------------------------------------------------------

        int luaopen_exec(lua_State* L);

    }
}
