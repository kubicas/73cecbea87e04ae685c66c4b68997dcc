#ifndef CPPMAKE__DEPGRAPH_CONFIG_H
#define CPPMAKE__DEPGRAPH_CONFIG_H

#include "path.h"
#include "process_config.h"
#include "target.h"

#include <memory>
#include <exception>
#include <iosfwd>
#include <functional>

namespace cppmake {

using on_target_t = std::function<void(target_t& target)>;

class depgraph_config_t
{
public:
    virtual ~depgraph_config_t() = 0;
    virtual path_t const& add_path(path_t const& path) = 0;
    virtual process_config_t& add_process(std::unique_ptr<process_config_t>&& process) = 0;
    virtual target_t const& add_target(std::string name, platform_t host_platform, toolset_t toolset, platform_t tgt_platform, config_t config) = 0;
    virtual void read(std::streambuf& streami) = 0;
    virtual void increase_run() = 0;
    virtual int& ide_filter() = 0;
    virtual void write(std::streambuf& streamo) const = 0;
    virtual void visit_targets(on_target_t on_target) = 0;
    virtual target_t* find_target(std::string const& name, platform_t host_platform, toolset_t toolset, platform_t tgt_platform, config_t config) = 0;
};

inline depgraph_config_t::~depgraph_config_t() {}

std::unique_ptr<cppmake::depgraph_config_t> create_empty_dependency_graph_config();

}; // namespace cppmake

#endif // CPPMAKE__DEPGRAPH_CONFIG_H
