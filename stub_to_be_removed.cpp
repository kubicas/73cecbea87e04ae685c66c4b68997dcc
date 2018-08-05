#include "depgraph_config.h"
#include "depgraph_exe.h"
#include "process_config.h"
#include "process_exe.h"
#include "bytestream/bytestream.h"
#include "bytestream/string.h"
#include "bytestream/set.h"
#include "bytestream/vector.h"
#include "bytestream/unique_ptr.h"
#include "bytestream/filesystem_path.h"
#include "exception/exception.h"

#include "stb/thread.h" // stubbable version of <thread>
#include <random>
#include <vector>
#include <set>
#include <ios>
#include <cstdint>
#include <utility>
#include <type_traits>

// macro to interpret enum as underlying type
#define TO_UNDERLYING(V) (*(std::underlying_type_t<decltype(V)>*)(&V))

namespace cppmake
{
template<typename B, BYTESTREAM_ENABLE_IF_IS_BYTESTREAM_BUF(B)>
inline B& operator||(B& streambuf, BYTESTREAM_PARAMETER_TYPE(B, process_config_t)& process);
}; // cppmake

namespace // anonymous
{

struct path_t
    : cppmake::path_t
{
    path_t()
    {}
    explicit path_t(stb::filesystem::path const& path)
        : m_path(path)
    {}
    bool operator<(path_t const& rhs) const
    {
        return m_path < rhs.m_path;
    }
    stb::filesystem::path m_path;
};

template<typename B, BYTESTREAM_ENABLE_IF_IS_BYTESTREAM_BUF(B)>
inline B& operator||(B& streambuf, BYTESTREAM_PARAMETER_TYPE(B, path_t)& path)
{
    return streambuf || path.m_path;
}

struct process_t
    : public cppmake::process_config_t
    , public cppmake::process_exe_t
{
    process_t()
        : m_executed(false)
        , m_id(-1)
        , m_abort(false)
        , m_pdefinition(nullptr)
        , m_ptarget(nullptr)
        , m_plogger(nullptr)
    {}
    process_t(
        cppmake::process_definition_t const& definition,
        cppmake::target_t const& target,
        cppmake::logger_t& logger)
        : m_executed(false)
        , m_run(-1)
        , m_id(-1)
        , m_abort(false)
        , m_pdefinition(&definition)
        , m_ptarget(&target)
        , m_plogger(&logger)
    {}
    process_t(process_t&& rhs) = default;
    void execute(int run, int id, cppmake::process_finished_handler_t& process_finished_handler) override
    {
        std::unique_ptr<void, std::function<void(void*)>> finally(reinterpret_cast<void*>(1), [&](void*)
        {   // called when leaving the scope, also after an exception
            process_finished_handler.on_finished(*this, !m_abort);
        });
        if (!m_plogger)
        {
            THROW(std::logic_error, << "No logger");
        }
        m_run = run;
        m_id = id;
        std::uniform_real_distribution<double> distribution(0.0, 0.01);
        for (int i = 0; i < 5000; ++i)
        {
            if (m_abort)
            {
                break;
            }
            stb::this_thread::sleep_for(std::chrono::duration<double>(distribution(m_generator)));
            if(m_plogger)
            m_plogger->on_log_line(*this, stb::filesystem::path("path"), 666, -1, cppmake::msg_t::error, std::string("C2660"), std::string("Waited some time"));
        }
        m_executed = true;
    }
    void abort() override
    {
        m_abort = true;
    }
    bool executed() const override
    {
        return m_executed;
    }
    int run() const override
    {
        return m_run;
    }
    int id() const override
    {
        return m_id;
    }
    stb::future<void>& future() override
    {
        return m_future;
    }
    // data to store
    bool m_executed;
    // runtime data
    int m_run;
    int m_id;
    bool m_abort;
    cppmake::process_definition_t const* m_pdefinition;
    cppmake::target_t const* m_ptarget;
    cppmake::logger_t* m_plogger;
    stb::future<void> m_future;
    static std::default_random_engine m_generator;
};

std::default_random_engine process_t::m_generator;

template<typename B, BYTESTREAM_ENABLE_IF_IS_BYTESTREAM_BUF(B)>
inline B& operator||(B& streambuf, BYTESTREAM_PARAMETER_TYPE(B, process_t)& process)
{
    return streambuf
        || process.m_executed;
}

struct target_t
    : cppmake::target_t
{
    target_t()
        : m_build(false)
    {}
    target_t(
        std::string name,
        cppmake::platform_t host_platform,
        cppmake::toolset_t toolset,
        cppmake::platform_t tgt_platform,
        cppmake::config_t config)
        : m_name(name)
        , m_host_platform(host_platform)
        , m_toolset(toolset)
        , m_tgt_platform(tgt_platform)
        , m_config(config)
        , m_build(false)
    {}
    std::string name() override
    {
        return m_name;
    }
    cppmake::platform_t host_platform() override
    {
        return m_host_platform;
    }
    cppmake::toolset_t toolset() override
    {
        return m_toolset;
    }
    cppmake::platform_t tgt_platform() override
    {
        return m_tgt_platform;
    }
    cppmake::config_t config() override
    {
        return m_config;
    }
    bool& build() override
    {
        return m_build;
    }
    bool operator<(target_t const& rhs) const
    {
        if (m_name < rhs.m_name) return true;
        if (rhs.m_name < m_name) return false;
        if (m_host_platform < rhs.m_host_platform) return true;
        if (rhs.m_host_platform < m_host_platform) return false;
        if (m_toolset < rhs.m_toolset) return true;
        if (rhs.m_toolset < m_toolset) return false;
        if (m_tgt_platform < rhs.m_tgt_platform) return true;
        if (rhs.m_tgt_platform < m_tgt_platform) return false;
        if (m_config < rhs.m_config) return true;
        if (rhs.m_config < m_config) return false;
        return false;
    }
    std::string m_name;
    cppmake::platform_t m_host_platform;
    cppmake::toolset_t m_toolset;
    cppmake::platform_t m_tgt_platform;
    cppmake::config_t m_config;
    bool m_build;
};

template<typename B, BYTESTREAM_ENABLE_IF_IS_BYTESTREAM_BUF(B)>
inline B& operator||(B& streambuf, BYTESTREAM_PARAMETER_TYPE(B, target_t)& target)
{
    return streambuf
        || target.m_name
        || TO_UNDERLYING(target.m_host_platform)
        || TO_UNDERLYING(target.m_toolset)
        || TO_UNDERLYING(target.m_tgt_platform)
        || TO_UNDERLYING(target.m_config)
        || target.m_build;
}

struct dependency_graph_t
    : cppmake::depgraph_config_t
    , cppmake::depgraph_exe_t

{
    dependency_graph_t()
    : m_run(0)
    , m_ide_filter(-1)
    , m_started(false)
    { }
    cppmake::path_t const& add_path(cppmake::path_t const& path) override
    {
        path_t const& path_downcasted = dynamic_cast<path_t const&>(path);
        return *m_paths.insert(path_downcasted).first;
    }
    cppmake::process_config_t& add_process(std::unique_ptr<cppmake::process_config_t>&& process) override
    {
        return *m_processes.emplace_back(std::move(process)).get();
    }
    cppmake::target_t const& add_target(std::string name, cppmake::platform_t host_platform, cppmake::toolset_t toolset, cppmake::platform_t tgt_platform, cppmake::config_t config) override
    {
        return *m_targets.emplace(std::move(target_t(name, host_platform, toolset, tgt_platform, config))).first;
    }
    void read(std::streambuf& streambuf) override;
    void increase_run() override
    {
        ++m_run;
    }
    int& ide_filter() override
    {
        return m_ide_filter;
    }
    void write(std::streambuf& streambuf) const override;
    cppmake::process_exe_t* get_next_process_to_start() override
    {
        if (!m_started)
        {
            m_pprocess = m_processes.begin();
            m_started = true;
        }
        if (m_pprocess == m_processes.end())
        {
            return nullptr;
        }
        else
        {
            return &dynamic_cast<cppmake::process_exe_t&>(**(m_pprocess++));
        }
    }
    void process_done(cppmake::process_exe_t* process, bool succes) override
    {}
    void visit_targets(cppmake::on_target_t on_target) override
    {
        for (target_t const& target : m_targets)
        {
            on_target(const_cast<target_t&>(target));
        }
    }
    cppmake::target_t* find_target(std::string const& name, cppmake::platform_t host_platform, cppmake::toolset_t toolset, cppmake::platform_t tgt_platform, cppmake::config_t config) override
    {
        targets_t::iterator ptarget = m_targets.find(target_t(name, host_platform, toolset, tgt_platform, config));
        if (ptarget == m_targets.end())
        {
            return nullptr;
        }
        else
        {
            return &const_cast<target_t&>(*ptarget);
        }
    }
    int run() override
    {
        return m_run;
    }
    using processes_t = std::vector<std::unique_ptr<cppmake::process_config_t>>;
    using paths_t = std::set<path_t>;
    using targets_t = std::set<target_t>;
    // data to store
    int m_run;
    int m_ide_filter;
    processes_t m_processes;
    paths_t m_paths;
    targets_t m_targets;
    // runtime data
    bool m_started;
    processes_t::iterator m_pprocess;
    uint64_t const m_magic_begin = 0xacf7ad8f511e4dd3;
    uint64_t const m_magic_end = 0xa90a2b0d1e32ef53;
    uint64_t const m_version = 0;
};

template<typename B, BYTESTREAM_ENABLE_IF_IS_BYTESTREAM_BUF(B)>
inline B& operator||(B& streambuf, BYTESTREAM_PARAMETER_TYPE(B, dependency_graph_t)& depgraph)
{
    streambuf 
        || depgraph.m_run
        || depgraph.m_ide_filter
        || depgraph.m_processes
        || depgraph.m_paths
        || depgraph.m_targets;
    return streambuf;
}

void dependency_graph_t::read(std::streambuf& streambuf)
{
    bytestream::istreambuf_t& istreambuf(*reinterpret_cast<bytestream::istreambuf_t*>(&streambuf));
    uint64_t uint64;
    istreambuf || uint64;
    if (uint64 != m_magic_begin)
    {
        THROW(std::runtime_error, << "Begin of binary make data is corrupt");
    }
    istreambuf || uint64;
    if (uint64 != m_version)
    {
        THROW(std::runtime_error, << "Version mismatch of binary make data");
    }
    istreambuf || *this || uint64;
    if (uint64 != m_magic_end)
    {
        THROW(std::runtime_error, << "End of binary make data is corrupt");
    }
}

void dependency_graph_t::write(std::streambuf& streambuf) const
{
    bytestream::ostreambuf_t& ostreambuf(*reinterpret_cast<bytestream::ostreambuf_t*>(&streambuf));
    ostreambuf
        || m_magic_begin
        || m_version
        || *this
        || m_magic_end;
}


}; // namespace anonymous

namespace cppmake
{

template<typename B, BYTESTREAM_ENABLE_IF_IS_BYTESTREAM_BUF_DEFINITION(B)>
inline B& operator||(B& streambuf, BYTESTREAM_PARAMETER_TYPE(B, process_config_t)& process)
{
    return streambuf || dynamic_cast<BYTESTREAM_PARAMETER_TYPE(B, process_t)&>(process);
}

std::unique_ptr<process_config_t> create_process(
    cppmake::process_definition_t const& definition, 
    cppmake::target_t const& target, 
    cppmake::logger_t& logger)
{
    return std::make_unique<::process_t>(definition, target, logger);
}

void make_unique(std::unique_ptr<process_config_t>& pprocess)
{
    pprocess = std::make_unique<::process_t>();
}

std::unique_ptr<path_t> create_path(stb::filesystem::path const& path)
{
    return std::make_unique<::path_t>(path);
}

std::unique_ptr<depgraph_exe_t> create_empty_dependency_graph_exe()
{
    return std::make_unique<::dependency_graph_t>();
}

depgraph_config_t& cast(depgraph_exe_t& depgraph)
{
    return dynamic_cast<depgraph_config_t&>(depgraph);
}

std::unique_ptr<cppmake::depgraph_config_t> create_empty_dependency_graph_config()
{
    return std::make_unique<::dependency_graph_t>();
}


}; // namespace cppmake
