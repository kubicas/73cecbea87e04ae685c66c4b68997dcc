#ifndef STUB_DEPGRAPH_EXE_H
#define STUB_DEPGRAPH_EXE_H

#include "test/test.h"
#include "test/stub.h"
#include "depgraph_exe.h"

namespace cppmake {

class depgraph_config_t {};

}; // namespace cppmake

struct stub_depgraph_exe_t
    : cppmake::depgraph_exe_t
    , cppmake::depgraph_config_t
{
    using get_next_process_to_start_t = cppmake::process_exe_t*();
    using process_done_t = void(cppmake::process_exe_t* process, bool success);
    using run_t = int();

    test::function_fifo_t<get_next_process_to_start_t> m_get_next_process_to_start;
    test::function_fifo_t<process_done_t> m_process_done;
    test::function_fifo_t<run_t> m_run;

    stub_depgraph_exe_t(
        std::ostream& os,
        int& count)
        : m_os(os)
        , m_count(count)
    {}

    cppmake::process_exe_t* get_next_process_to_start() override
    {
        if (m_get_next_process_to_start.empty()) ASSERT_STUB_IS_NOT_EMPTY;
        return test::get(m_get_next_process_to_start)();
    }
    void process_done(cppmake::process_exe_t* process, bool success) override
    {
        if (m_process_done.empty()) ASSERT_STUB_IS_NOT_EMPTY;
        test::get(m_process_done)(process, success);
    }
    int run() override
    {
        if (m_run.empty()) ASSERT_STUB_IS_NOT_EMPTY;
        return test::get(m_run)();
    }

    ~stub_depgraph_exe_t()
    {
        if (!m_get_next_process_to_start.empty()) EXPECT_EMPTY_STUB(m_os, m_count);
        if (!m_process_done.empty()) EXPECT_EMPTY_STUB(m_os, m_count);
        if (!m_run.empty()) EXPECT_EMPTY_STUB(m_os, m_count);
    }

    std::ostream& m_os;
    int& m_count;
};

#endif // STUB_DEPGRAPH_EXE_H