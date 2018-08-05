#ifndef CPPMAKE__PROCESS_EXE_H
#define CPPMAKE__PROCESS_EXE_H

#include "stb/future.h"

namespace cppmake {

class process_exe_t;

class process_finished_handler_t
{
public:
    virtual ~process_finished_handler_t() = 0;
    virtual void on_finished(process_exe_t& process, bool succes) = 0;
};
inline process_finished_handler_t::~process_finished_handler_t() {}

class process_exe_t
{
public:
    virtual ~process_exe_t() = 0;
    virtual void execute(int run, int id, cppmake::process_finished_handler_t& process_finished_handler) = 0;
    virtual void abort() = 0;
    virtual int id() const = 0;
    virtual stb::future<void>& future() = 0;
};
inline process_exe_t::~process_exe_t() {}

}; // namespace cppmake

#endif // CPPMAKE__PROCESS_EXE_H
