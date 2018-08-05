#ifndef STUB_PROCESS_EXE_H
#define STUB_PROCESS_EXE_H

#include "test/test.h"
#include "test/stub.h"
#include "process_exe.h"

struct stub_process_exe_t
    : cppmake::process_exe_t
{
    stub_process_exe_t(
        std::ostream& os,
        int& count)
        : m_os(os)
        , m_count(count)
        , m_id(0)
    {}

    using execute_t = void(int run, int id, cppmake::process_finished_handler_t& process_finished_handler);
    using abort_t = void();

    test::function_fifo_t<execute_t> m_execute;
    test::function_fifo_t<abort_t> m_abort;

    void execute(int run, int id, cppmake::process_finished_handler_t& process_finished_handler) override
    {
        m_id = id;
        if (m_execute.empty()) ASSERT_STUB_IS_NOT_EMPTY;
        test::get(m_execute)(run, id, process_finished_handler);
    }
    void abort() override
    {
        if (m_abort.empty()) ASSERT_STUB_IS_NOT_EMPTY;
        test::get(m_abort)();
    }
    int id() const override
    {
        return m_id;
    }
    stb::future<void>& future() override
    {
        return m_future;
    }

    ~stub_process_exe_t()
    {
        if (!m_execute.empty()) EXPECT_EMPTY_STUB(m_os, m_count);
        if (!m_abort.empty()) EXPECT_EMPTY_STUB(m_os, m_count);
    }

    std::ostream& m_os;
    int& m_count;
    int m_id;
    stb::future<void> m_future;
};

#endif // STUB_PROCESS_EXE_H