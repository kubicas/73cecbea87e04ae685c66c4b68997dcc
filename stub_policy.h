#ifndef STUB_POLICY_H
#define STUB_POLICY_H

#include "test/test.h"
#include "test/stub.h"
#include "policy.h"

struct stub_policy_t
    : cppmake::policy_t
{
    stub_policy_t(
        std::ostream& os,
        int& count,
        cppmake::depgraph_config_t& depgraph,
        std::set<std::string> const& arguments, 
        cppmake::thread_safe_console_t& thread_safe_console)
        : m_os(os)
        , m_count(count)
        , m_depgraph(depgraph)
        , m_arguments(arguments)
        , m_thread_safe_console(thread_safe_console)
    {}
    cppmake::depgraph_config_t& m_depgraph;
    std::set<std::string> const& m_arguments;
    cppmake::thread_safe_console_t& m_thread_safe_console;

    using finish_t = bool(bool& rerun);
    using build_t = bool();

    test::function_fifo_t<finish_t> m_finish;
    test::function_fifo_t<build_t> m_build;

    bool finish(bool& rerun) override
    {
        if (m_finish.empty()) ASSERT_STUB_IS_NOT_EMPTY;
        return test::get(m_finish)(rerun);
    }
    bool build() override
    {
        if (m_build.empty()) ASSERT_STUB_IS_NOT_EMPTY;
        return test::get(m_build)();
    }

    ~stub_policy_t()
    {
        if (!m_finish.empty()) EXPECT_EMPTY_STUB(m_os, m_count);
        if (!m_build.empty()) EXPECT_EMPTY_STUB(m_os, m_count);
    }

    std::ostream& m_os;
    int& m_count;
};

#endif // STUB_POLICY_H