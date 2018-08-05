#ifndef CPPMAKE__POLICY_H
#define CPPMAKE__POLICY_H

//#include "depgraph_config.h"

#include <memory>
#include <set>
#include <string>

namespace cppmake {

class thread_safe_console_t
{
public:
    virtual ~thread_safe_console_t() = 0;
    virtual void print_line(std::string const& text) = 0;
};
inline thread_safe_console_t::~thread_safe_console_t() {}

class policy_t
{
public:
    virtual ~policy_t() = 0;
    virtual bool finish(bool& rerun) = 0;
    virtual bool build() = 0;
};
inline policy_t::~policy_t() {}

class depgraph_config_t;

std::unique_ptr<policy_t> create_policy(depgraph_config_t& depgraph, std::set<std::string> const& arguments, thread_safe_console_t& thread_safe_console);
bool handle_exception(cppmake::policy_t* ppolicy, std::exception& exc);

}; // namespace cppmake

#endif // CPPMAKE__POLICY_H
