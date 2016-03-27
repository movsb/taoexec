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

        public:
            executor_fs();

            const std::string get_name() const override {
                return "fs";
            }

            bool execute(const std::string& args) override;

        private:
            void _expand_exec(const std::string& newcmd, const std::vector<std::string>& argv, std::string* __argstr);

            // 变量展开
            // 支持的替换：$foo() - 函数调用，${variable} - 变量展开，${number} - 基于位置的变量展开
            void _expand_args(const std::string& cmd, const std::vector<std::string>& argv, std::string* __newcmd);

            // 基于词法规则的参数分隔函数
            // 单词分隔符：不处在引号中的 <space> <tab>。
            int _split_args(const std::string& args, std::vector<std::string>* __argv);


            void _initialize_globals();
            void _add_user_variables(const env_var_t& env_var);
            std::string _expand_variable(const std::string& var);
            std::string _expand_function(const std::string& fn, func_args& args);
            std::string _which(const std::string& cmd, const std::string& env);
            void explorer(HWND hwnd, const std::string& path, std::function<void(const std::string& err)> cb);

            class path_info_t
            {
            public:
                enum class type_t {
                    null,
                    path,
                    sharing,
                    protocol,
                };

                type_t type;

                union {
                    struct {
                        std::string path;
                    };
                    struct {
                        std::string scheme;
                        std::string spec;
                    };
                    struct {
                        std::string path;
                    };
                };
            };

            void get_pathinfo(const std::string& path, path_info_t* ppi) {
                if (std::regex_match(path, std::regex(R"([0-9a-zA-z]+:.*)", std::regex_constants::icase))) {
                    ppi->type = path_info_t::type_t::protocol;
                }
                    
            }

            bool execute(HWND hwnd, const std::string& path,
                const std::string& params, const std::string& args,
                const std::string& wd_, const std::string& env_,
                std::function<void(const std::string& err)> cb);

            void execute(HWND hwnd, const std::vector<std::string>& paths,
                const std::string& params, const std::string& args,
                const std::string& wd_, const std::string& env_,
                std::function<void(const std::string& err)> cb);

            std::string get_executor(const std::string& ext);

            void init() {
                _initialize_globals();
            }

            void uninit() {

            }
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
    }
}
