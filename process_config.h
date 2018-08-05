#ifndef CPPMAKE__PROCESS_CONFIG_H
#define CPPMAKE__PROCESS_CONFIG_H

#include "target.h"
#include "logger.h"

#include <map>
#include <string>
#include "stb/filesystem.h" // stubbable version of <filesystem>

namespace cppmake {

using environment_t = std::map<std::string, std::string>;

class process_definition_t
{
public:
    virtual ~process_definition_t() = 0;
    virtual stb::filesystem::path on_application(target_t const& target) const = 0;
    virtual stb::filesystem::path on_home_path(target_t const& target) const = 0;
    virtual environment_t const& on_environment(target_t const& target) const = 0;
    virtual std::string on_parameters(target_t const& target) const = 0;
};
inline process_definition_t::~process_definition_t() {}

class process_config_t
{
public:
    virtual ~process_config_t() = 0;
    virtual int run() const = 0;
    virtual int id() const = 0;
    virtual bool executed() const = 0;
};
inline process_config_t::~process_config_t() {}

std::unique_ptr<process_config_t> create_process(process_definition_t const& definition, target_t const& target, logger_t& logger);
void make_unique(std::unique_ptr<process_config_t>& pprocess); // for istreambuf streaming

}; // namespace cppmake

#endif // CPPMAKE__PROCESS_CONFIG_H
