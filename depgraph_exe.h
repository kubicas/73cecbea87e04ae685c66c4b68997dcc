#ifndef CPPMAKE__DEPGRAPH_EXE_H
#define CPPMAKE__DEPGRAPH_EXE_H

#include <memory>

namespace cppmake {

class process_exe_t;
class depgraph_config_t;

class depgraph_exe_t
{
public:
    virtual ~depgraph_exe_t() = 0;
    virtual process_exe_t* get_next_process_to_start() = 0;
    virtual void process_done(process_exe_t* process, bool succes) = 0;
    virtual int run() = 0;
};

inline depgraph_exe_t::~depgraph_exe_t() {}

std::unique_ptr<depgraph_exe_t> create_empty_dependency_graph_exe();
depgraph_config_t& cast(depgraph_exe_t& depgraph);

}; // namespace cppmake

#endif // CPPMAKE__DEPGRAPH_EXE_H
