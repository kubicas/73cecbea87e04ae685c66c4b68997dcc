#include "policy.h"
#include "ide_filter.h"
#include "process_config.h"
#include "depgraph_config.h"
#include "target.h"

#include "stb/fstream.h" // stubbable version of <fstream>
#include "stb/filesystem.h" // stubbable version of <filesystem>
#include "exception/exception.h"
#include "cppmake/makefile.h"
#include <set>
#include <string>
#include <sstream>

namespace cppmake // because of argument dependent lookup
{
#define OS_ENUM(type,item) case type##_t::item: return os << #type "::" #item
#define IS_ENUM(type,item) if (token == #type "::" #item) { value = type##_t::item; return is; }

static inline std::ostream& operator<<(std::ostream& os, cppmake::platform_t value)
{
    switch (value)
    {
        OS_ENUM(platform, unknown);
        OS_ENUM(platform, winx86);
        OS_ENUM(platform, winx64);
        OS_ENUM(platform, linux);
        OS_ENUM(platform, osx);
    }
    THROW_INVALID_ENUM(std::runtime_error, cppmake::platform_t);
}

static inline std::istream& operator>>(std::istream& is, cppmake::platform_t& value)
{
    std::string token;
    is >> token;
    IS_ENUM(platform, winx86);
    IS_ENUM(platform, winx64);
    IS_ENUM(platform, linux);
    IS_ENUM(platform, osx);
    value = cppmake::platform_t::unknown;
    return is;
}

static inline std::ostream& operator<<(std::ostream& os, cppmake::toolset_t value)
{
    switch (value)
    {
        OS_ENUM(toolset, unknown);
        OS_ENUM(toolset, ms);
        OS_ENUM(toolset, clang);
        OS_ENUM(toolset, gcc);
    }
    THROW_INVALID_ENUM(std::runtime_error, cppmake::toolset_t);
}

static inline std::istream& operator>>(std::istream& is, cppmake::toolset_t& value)
{
    std::string token;
    is >> token;
    IS_ENUM(toolset, ms);
    IS_ENUM(toolset, clang);
    IS_ENUM(toolset, gcc);
    value = cppmake::toolset_t::unknown;
    return is;
}

static inline std::ostream& operator<<(std::ostream& os, cppmake::config_t value)
{
    switch (value)
    {
        OS_ENUM(config, unknown);
        OS_ENUM(config, release);
        OS_ENUM(config, debug);
    }
    THROW_INVALID_ENUM(std::runtime_error, cppmake::config_t);
}

static inline std::istream& operator>>(std::istream& is, cppmake::config_t& value)
{
    std::string token;
    is >> token;
    IS_ENUM(config, release);
    IS_ENUM(config, debug);
    value = cppmake::config_t::unknown;
    return is;
}

static inline std::ostream& operator<<(std::ostream& os, cppmake::msg_t value)
{
    switch (value)
    {
        OS_ENUM(msg, unknown);
        OS_ENUM(msg, error);
        OS_ENUM(msg, warning);
        OS_ENUM(msg, note);
    }
    THROW_INVALID_ENUM(std::runtime_error, cppmake::msg_t);
}

static inline std::istream& operator>>(std::istream& is, cppmake::msg_t& value)
{
    std::string token;
    is >> token;
    IS_ENUM(msg, error);
    IS_ENUM(msg, warning);
    IS_ENUM(msg, note);
    value = cppmake::msg_t::unknown;
    return is;
}

#undef OS_ENUM
#undef IS_ENUM
};

namespace // anonymous
{

class policy_t
    : public cppmake::policy_t
    , public cppmake::logger_t
    , public cppmake::process_definition_t
{
public:
    policy_t(cppmake::depgraph_config_t& depgraph, std::set<std::string> const& arguments, cppmake::thread_safe_console_t& thread_safe_console)
        : m_build(false)
        , m_depgraph(depgraph)
        , m_thread_safe_console(thread_safe_console)
    {
        m_makedbpath = stb::filesystem::path("C:/Users/310192535/Documents/Visual Studio 2017/Projects/cppmake/make.bin");
        std::unique_ptr<cppmake::depgraph_config_t> pformer_depgraph = read_former_depgraph();
        pformer_depgraph->increase_run();
        m_pide_filter = cppmake::create_ide_filter(static_cast<cppmake::ide_t>(m_depgraph.ide_filter()));

        // dit moet in de make file eindigen ....
        //    depgraph.add_path(...);
        cppmake::target_t const& target = depgraph.add_target("mytarget", cppmake::platform_t::winx86, cppmake::toolset_t::ms, cppmake::platform_t::winx86, cppmake::config_t::release);
        std::unique_ptr<cppmake::process_config_t> process = cppmake::create_process(*this, target, *this);
        m_depgraph.add_process(std::move(process));
        process = cppmake::create_process(*this, target, *this);
        m_depgraph.add_process(std::move(process));
        process = cppmake::create_process(*this, target, *this);
        m_depgraph.add_process(std::move(process));
        process = cppmake::create_process(*this, target, *this);
        m_depgraph.add_process(std::move(process));
        process = cppmake::create_process(*this, target, *this);
        m_depgraph.add_process(std::move(process));

        unsigned int command = 0;
        unsigned int const command_info  = 1 << 0;
        unsigned int const command_set   = 1 << 1;
        unsigned int const command_reset = 1 << 2;
        command |= arguments.count("info")  ? command_info  : 0;
        command |= arguments.count("set")   ? command_set   : 0;
        command |= arguments.count("reset") ? command_reset : 0;

        switch(command)
        {
        case 0:
            m_build = true;
            break;
        case command_info:
            info(arguments);
            break;
        case command_set:
            set(arguments, true);
            break;
        case command_reset:
            set(arguments, false);
            break;
        default:
            THROW(std::runtime_error, << "Conflicting commands on the commandline");
        }
    }
    bool build() override
    {
        return m_build;
    }
    bool finish(bool& rerun) override
    {
        write_current_depgraph(m_depgraph);
        unlock();
        rerun = false;
        return true /*ok*/;
    }
    void handle_exception(std::exception& exc)
    {
        std::stringstream ss;
        ss << "Exception: " << exc.what();
        m_thread_safe_console.print_line(ss.str());
        unlock();
    }

private:

    void on_log_line(cppmake::process_config_t const& process, stb::filesystem::path const& path, int line, int column, cppmake::msg_t msg, std::string const& msg_id, std::string const& text) override
    {
        std::string log_line = m_pide_filter->to_string(process.id(), process.run(), path, line, column, msg, msg_id, text);
        m_thread_safe_console.print_line(log_line);
    }
    void on_log_line(cppmake::process_config_t const& process, cppmake::msg_t msg, std::string const& msg_id, std::string const& text) override
    {
        std::string log_line = m_pide_filter->to_string(process.id(), process.run(), msg, msg_id, text);
        m_thread_safe_console.print_line(log_line);
    }
    void on_log_line(cppmake::process_config_t const& process, stb::filesystem::path const& path, int line, int column, cppmake::msg_t msg, std::string const& text) override
    {
        std::string log_line = m_pide_filter->to_string(process.id(), process.run(), path, line, column, msg, text);
        m_thread_safe_console.print_line(log_line);
    }
    void on_log_line(cppmake::process_config_t const& process, cppmake::msg_t msg, std::string const& text) override
    {
        std::string log_line = m_pide_filter->to_string(process.id(), process.run(), msg, text);
        m_thread_safe_console.print_line(log_line);
    }
    stb::filesystem::path on_application(cppmake::target_t const& /*target*/) const override
    {
        return stb::filesystem::path("application_path");
    }
    stb::filesystem::path on_home_path(cppmake::target_t const& /*target*/) const override
    {
        return stb::filesystem::path("home_path");
    }
    cppmake::environment_t const& on_environment(cppmake::target_t const& /*target*/) const override
    {
        static cppmake::environment_t environment;
        return environment;
    }
    std::string on_parameters(cppmake::target_t const& /*target*/) const override
    {
        return std::string("parameters");
    }

    bool lock()
    {
        // not implemented
        return true;
    }

    void unlock()
    {
        // not implemented
    }

    std::unique_ptr<cppmake::depgraph_config_t> read_former_depgraph()
    {
        if (!lock())
        {
            THROW(std::runtime_error, << "Already an instance of cppmake is running");
        }
        std::unique_ptr<cppmake::depgraph_config_t> pformer_depgraph = cppmake::create_empty_dependency_graph_config();
        stb::filebuf former_depgraph_file;
        former_depgraph_file.open(m_makedbpath, std::ios::in | std::ios::binary);
        if (former_depgraph_file.is_open())
        {
            pformer_depgraph->read(former_depgraph_file);
        }
        return pformer_depgraph;
    }

    void write_current_depgraph(cppmake::depgraph_config_t& depgraph)
    {
        stb::filebuf depgraph_file;
        depgraph_file.open(m_makedbpath, std::ios::out | std::ios::binary);
        if (depgraph_file.is_open())
        {
            depgraph.write(depgraph_file);
        }
    }

    void info(std::set<std::string> const& arguments)
    {
        if (arguments.count("targets"))
        {
            info_list_targets();
        }
    }

    void info_list_targets()
    {
        auto on_target = [this](cppmake::target_t& target)
        {
            std::stringstream ss;
            ss  << target.name() << '-' 
                << target.host_platform() << '-' 
                << target.toolset() << '-' 
                << target.tgt_platform() << '-' 
                << target.config() << "  ->  "
                << (target.build() ? "build" : "skip") << std::endl;
            m_thread_safe_console.print_line(ss.str());
        };
        m_depgraph.visit_targets(on_target);
    }

    void set(std::set<std::string> const& arguments, bool build)
    {
        for (std::string const& argument : arguments)
        {
            std::stringstream argumentss(argument);
            std::string token;
            std::string         name;          std::getline(argumentss, token, '-'); name = token;
            cppmake::platform_t host_platform; std::getline(argumentss, token, '-'); std::stringstream(token) >> host_platform;
            cppmake::toolset_t  toolset;       std::getline(argumentss, token, '-'); std::stringstream(token) >> toolset;
            cppmake::platform_t tgt_platform;  std::getline(argumentss, token, '-'); std::stringstream(token) >> tgt_platform;
            cppmake::config_t   config;        std::getline(argumentss, token);      std::stringstream(token) >> config;
            if (cppmake::target_t* target = m_depgraph.find_target(name, host_platform, toolset, tgt_platform, config))
            {
                target->build() = build;
            }
        }
    }

    bool m_build;
    cppmake::depgraph_config_t& m_depgraph;
    stb::filesystem::path m_makedbpath;
    std::unique_ptr<cppmake::ide_filter_t> m_pide_filter;
    cppmake::thread_safe_console_t& m_thread_safe_console;
};


};

namespace cppmake
{
static cppmake::main_t* mains = nullptr;

main_t::main_t()
	: m_next(mains)
{
	mains = this;
}

void run()
{
	for (cppmake::main_t* it = mains; it; it = it->m_next) { it->provides(); }
	for (cppmake::main_t* it = mains; it; it = it->m_next) { it->requires(); }
	for (cppmake::main_t* it = mains; it; it = it->m_next) { it->contains(); }
}

std::unique_ptr<policy_t> create_policy(depgraph_config_t& depgraph, std::set<std::string> const& arguments, thread_safe_console_t& thread_safe_console)
{
	run();
	return std::make_unique<::policy_t>(depgraph, arguments, thread_safe_console);
}


bool handle_exception(policy_t* ppolicy, std::exception& exc)
{
    ::policy_t* ppolicy_downcasted = dynamic_cast<::policy_t*>(ppolicy);
    if (ppolicy_downcasted)
    {
        ppolicy_downcasted->handle_exception(exc);
        return true;
    }
    return false;
}

}; // namespace cppmake
