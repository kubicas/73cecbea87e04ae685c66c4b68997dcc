#include "test/test.h"
#include "stub_depgraph_exe.h"
#include "stub_policy.h"
#include "stub_process_exe.h"

#include "stb/filesystem.h"
#include <iostream>
#include <sstream>
#include <mutex>

extern int execute(int argc, char const* argv[], stb::filesystem::path const& group, int run_count);
extern void abort_execution();

using create_empty_dependency_graph_exe_t = std::unique_ptr<cppmake::depgraph_exe_t>();
using create_policy_t = std::unique_ptr<cppmake::policy_t>(cppmake::depgraph_config_t& depgraph, std::set<std::string> const& arguments, cppmake::thread_safe_console_t& thread_safe_console);
using handle_exception_t = bool(cppmake::policy_t* ppolicy, std::exception& exc);

test::function_fifo_t<create_empty_dependency_graph_exe_t> g_create_empty_dependency_graph_exe;
test::function_fifo_t<create_policy_t> g_create_policy;
test::function_fifo_t<handle_exception_t> g_handle_exception;

void assert_all_empty(
    std::ostream& os,
    int& count)
{
    if (!g_create_empty_dependency_graph_exe.empty()) EXPECT_EMPTY_STUB(os,count);
    if (!g_create_policy.empty()) EXPECT_EMPTY_STUB(os, count);
    if (!g_handle_exception.empty()) EXPECT_EMPTY_STUB(os, count);
}

namespace cppmake {

std::unique_ptr<depgraph_exe_t> create_empty_dependency_graph_exe()
{
    if (g_create_empty_dependency_graph_exe.empty()) ASSERT_STUB_IS_NOT_EMPTY;
    return test::get(g_create_empty_dependency_graph_exe)();
}

depgraph_config_t& cast(depgraph_exe_t& depgraph)
{
    return dynamic_cast<depgraph_config_t&>(depgraph);
}

std::unique_ptr<policy_t> create_policy(depgraph_config_t& depgraph, std::set<std::string> const& arguments, thread_safe_console_t& thread_safe_console)
{
    if (g_create_policy.empty()) ASSERT_STUB_IS_NOT_EMPTY;
    return test::get(g_create_policy)(depgraph, arguments, thread_safe_console);
}

bool handle_exception(cppmake::policy_t* ppolicy, std::exception& exc)
{
    if (g_handle_exception.empty()) ASSERT_STUB_IS_NOT_EMPTY;
    return test::get(g_handle_exception)(ppolicy, exc);
}

}; // namespace cppmake

/******************************************************************************
helper function
******************************************************************************/
void create_dependency_graph_exe_and_policy_1(
    std::ostream& os,
    int& count,
    std::function<stub_policy_t::build_t> const& policy__build,
    std::function<stub_policy_t::finish_t> const& policy__finish,
    std::set<std::string> const*& parguments,
    std::function<create_empty_dependency_graph_exe_t>& create_empty_dependency_graph_exe,
    std::function<create_policy_t>& create_policy,
    stub_depgraph_exe_t*& pstub_depgraph_exe,
    stub_policy_t*& pstub_policy,
    cppmake::thread_safe_console_t*& pthread_safe_console)
{
    create_empty_dependency_graph_exe = [&]()->std::unique_ptr<cppmake::depgraph_exe_t>
    {
        std::unique_ptr<stub_depgraph_exe_t> p = std::make_unique<stub_depgraph_exe_t>(os, count);
        pstub_depgraph_exe = p.get();
        return p;
    };
    create_policy = [&](
            cppmake::depgraph_config_t& depgraph,
            std::set<std::string> const& arguments,
            cppmake::thread_safe_console_t& thread_safe_console)
        ->std::unique_ptr<cppmake::policy_t>
    {
        parguments = &arguments;
        pthread_safe_console = &thread_safe_console;
        std::unique_ptr<stub_policy_t> p = std::make_unique<stub_policy_t>(
            os, count, depgraph, arguments, thread_safe_console);
        pstub_policy = p.get();
        p->m_build.push(policy__build);
        p->m_finish.push(policy__finish);
        return p;
    };
    g_create_empty_dependency_graph_exe.push(create_empty_dependency_graph_exe);
    g_create_policy.push(create_policy);
}

/******************************************************************************
helper function
******************************************************************************/
void create_dependency_graph_exe_and_policy_2(
    std::ostream& os,
    int& count,
    std::function<stub_depgraph_exe_t::get_next_process_to_start_t> const& depgraph_exe__get_next_process_to_start,
    std::function<stub_depgraph_exe_t::process_done_t> const& depgraph_exe__process_done,
    std::function<stub_depgraph_exe_t::run_t> const& depgraph_exe__run,
    std::function<stub_policy_t::finish_t> const& policy__finish,
    std::function<create_empty_dependency_graph_exe_t>& create_empty_dependency_graph_exe,
    std::function<create_policy_t>& create_policy,
    stub_depgraph_exe_t*& pstub_depgraph_exe,
    stub_policy_t*& pstub_policy)
{
    create_empty_dependency_graph_exe = [&]()->std::unique_ptr<cppmake::depgraph_exe_t>
    {
        std::unique_ptr<stub_depgraph_exe_t> p = std::make_unique<stub_depgraph_exe_t>(os, count);
        pstub_depgraph_exe = p.get();
        if (depgraph_exe__get_next_process_to_start)
        {
            p->m_get_next_process_to_start.push(depgraph_exe__get_next_process_to_start);
        }
        if (depgraph_exe__process_done)
        {
            p->m_process_done.push(depgraph_exe__process_done);
        }
        if (depgraph_exe__run)
        {
            p->m_run.push(depgraph_exe__run);
        }
        return p;
    };
    create_policy = [&](
        cppmake::depgraph_config_t& depgraph,
        std::set<std::string> const& arguments,
        cppmake::thread_safe_console_t& thread_safe_console)
        ->std::unique_ptr<cppmake::policy_t>
    {
        std::unique_ptr<stub_policy_t> p = std::make_unique<stub_policy_t>(
            os, count, depgraph, arguments, thread_safe_console);
        pstub_policy = p.get();
        p->m_build.push([&]()->bool
        {
            return true;
        });
        if (policy__finish)
        {
            p->m_finish.push(policy__finish);
        }
        return p;
    };
    g_create_empty_dependency_graph_exe.push(create_empty_dependency_graph_exe);
    g_create_policy.push(create_policy);
}

/******************************************************************************
nothing to build
parameter: finished_successfully
parameter: rerun
check return value of execute() is 'expect_return'
check also whether command-line parameters are passed to policy
RT.executor.return
******************************************************************************/
void nothing_to_build(std::ostream& os, int& count, 
    bool finished_successfully, bool rerun, int expect_return)
{
    os << "- " << __func__ 
        << " finished_successfully=" << finished_successfully 
        << " rerun=" << rerun
        << std::endl;
    stb::filesystem::path group("test_root/projects/cppmake");
    stub_depgraph_exe_t* pstub_depgraph_exe = nullptr;
    stub_policy_t* pstub_policy = nullptr;
    std::set<std::string> const* parguments = nullptr;
    cppmake::thread_safe_console_t* pthread_safe_console = nullptr;
    int const argc = 4;
    char const* argv[argc] = { "test_root/projects/cppmake/tgt/winx86r/cppmake.exe", "opt2", "opt1", "opt3" };
    auto go = [&]() -> void
    {
        int ret = execute(argc, argv, group, 0);
        EXPECT_COMP(os, count, ret, == , expect_return, << " finished_successfully=" << finished_successfully << " rerun=" << rerun);
        assert_all_empty(os, count);
    };
    std::function<create_empty_dependency_graph_exe_t> create_empty_dependency_graph_exe;
    std::function<create_policy_t> create_policy;
    std::function<stub_policy_t::build_t> policy__build;
    std::function<stub_policy_t::finish_t> policy__finish;
    create_dependency_graph_exe_and_policy_1(
        os,
        count,
        policy__build,
        policy__finish,
        parguments,
        create_empty_dependency_graph_exe,
        create_policy,
        pstub_depgraph_exe,
        pstub_policy,
        pthread_safe_console);
    policy__build = [&]()->bool
    {
        ASSERT_TRUE(parguments, << " finished_successfully=" << finished_successfully << " rerun=" << rerun);
        ASSERT_COMP(parguments->size(), == , argc, << " finished_successfully=" << finished_successfully << " rerun=" << rerun);
        EXPECT_TRUE(os, count, parguments->find(std::string("program_name=") + argv[0]) != parguments->end(), 
            << " finished_successfully=" << finished_successfully << " rerun=" << rerun);
        for (int i = 1; i <= 3; ++i)
        {
            EXPECT_TRUE(os, count, parguments->find(argv[i]) != parguments->end(),
                << " finished_successfully=" << finished_successfully << " rerun=" << rerun);
        }
        return false;
    };
    policy__finish = [&, r=rerun](bool& rerun)->bool
    {
        rerun = r;
        return finished_successfully;
    };
    go();
}

/******************************************************************************
print line on cout
RT.executor.build.threading
******************************************************************************/
void print_line_on_cout(std::ostream& os, int& count)
{
    os << "- " << __func__ << std::endl;
    stb::filesystem::path group("test_root/projects/cppmake");
    stub_depgraph_exe_t* pstub_depgraph_exe = nullptr;
    stub_policy_t* pstub_policy = nullptr;
    std::set<std::string> const* parguments = nullptr;
    cppmake::thread_safe_console_t* pthread_safe_console = nullptr;
    std::string const test_string("test_string");
    auto go = [&]() -> void
    {
        int ret = execute(/*argc=*/ 0, /*argv=*/ 0, group, 0);
        assert_all_empty(os, count);
    };
    std::function<create_empty_dependency_graph_exe_t> create_empty_dependency_graph_exe;
    std::function<create_policy_t> create_policy;
    std::function<stub_policy_t::build_t> policy__build;
    std::function<stub_policy_t::finish_t> policy__finish;
    create_dependency_graph_exe_and_policy_1(
        os,
        count,
        policy__build,
        policy__finish,
        parguments,
        create_empty_dependency_graph_exe,
        create_policy,
        pstub_depgraph_exe,
        pstub_policy,
        pthread_safe_console);

    policy__build = [&]()->bool
    {
        ASSERT_TRUE(pthread_safe_console,);
        std::stringstream ss;
        std::streambuf* old_buf = std::cout.rdbuf(ss.rdbuf());
        pthread_safe_console->print_line(test_string);
        std::cout.rdbuf(old_buf);
        ASSERT_COMP(ss.str(), == , test_string + '\n', );
        return false;
    };
    policy__finish = [&](bool& rerun)->bool
    {
        rerun = false;
        return /*finished_successfully=*/true;
    };
    go();
}

/******************************************************************************
do build but no work to do
******************************************************************************/
void do_build_but_no_work_to_do(std::ostream& os, int& count)
{
    os << "- " << __func__ << std::endl;
    stb::filesystem::path group("test_root/projects/cppmake");
    stub_depgraph_exe_t* pstub_depgraph_exe = nullptr;
    stub_policy_t* pstub_policy = nullptr;
    auto go = [&]() -> void
    {
        int ret = execute(0, 0, group, 0);
        assert_all_empty(os, count);
    };
    std::function<create_empty_dependency_graph_exe_t> create_empty_dependency_graph_exe;
    std::function<stub_depgraph_exe_t::get_next_process_to_start_t> depgraph_exe__get_next_process_to_start;
    std::function<stub_depgraph_exe_t::process_done_t> depgraph_exe__process_done;
    std::function<stub_depgraph_exe_t::run_t> depgraph_exe__run;
    std::function<stub_policy_t::finish_t> policy__finish;
    std::function<create_policy_t> create_policy;
    create_dependency_graph_exe_and_policy_2(
        os,
        count,
        depgraph_exe__get_next_process_to_start,
        depgraph_exe__process_done,
        depgraph_exe__run,
        policy__finish,
        create_empty_dependency_graph_exe,
        create_policy,
        pstub_depgraph_exe,
        pstub_policy);
    depgraph_exe__get_next_process_to_start = [&]()->cppmake::process_exe_t*
    {
        return nullptr;
    };
    policy__finish = [&](bool& rerun)->bool
    {
        rerun = false;
        return /*finished_successfully=*/true;
    };
    go();
}

/******************************************************************************
do build and do process
******************************************************************************/
void do_build_and_do_process(std::ostream& os, int& count)
{
    os << "- " << __func__ << std::endl;
    stb::filesystem::path group("test_root/projects/cppmake");
    stub_depgraph_exe_t* pstub_depgraph_exe = nullptr;
    stub_policy_t* pstub_policy = nullptr;
    stub_process_exe_t process_exe(os, count);
    int const test_run = 123;
    bool const test_success = true;
    auto go = [&]() -> void
    {
        int ret = execute(0, 0, group, 0);
        assert_all_empty(os, count);
    };
    std::function<create_empty_dependency_graph_exe_t> create_empty_dependency_graph_exe;
    std::function<stub_depgraph_exe_t::get_next_process_to_start_t> depgraph_exe__get_next_process_to_start;
    std::function<stub_depgraph_exe_t::process_done_t> depgraph_exe__process_done;
    std::function<stub_depgraph_exe_t::run_t> depgraph_exe__run;
    std::function<stub_policy_t::finish_t> policy__finish;
    std::function<create_policy_t> create_policy;
    create_dependency_graph_exe_and_policy_2(
        os,
        count,
        depgraph_exe__get_next_process_to_start,
        depgraph_exe__process_done,
        depgraph_exe__run,
        policy__finish,
        create_empty_dependency_graph_exe,
        create_policy,
        pstub_depgraph_exe,
        pstub_policy);
    depgraph_exe__get_next_process_to_start = [&]()->cppmake::process_exe_t*
    {
        // for next 2 times time
        std::function<stub_depgraph_exe_t::get_next_process_to_start_t> f = [&]()->cppmake::process_exe_t*
        {
            return nullptr;
        };
        pstub_depgraph_exe->m_get_next_process_to_start.push(f);
        pstub_depgraph_exe->m_get_next_process_to_start.push(f);
        return &process_exe;
    };
    depgraph_exe__run = [&]()->int
    {
        return test_run;
    };
    process_exe.m_execute.push([&](int run, int id, cppmake::process_finished_handler_t& process_finished_handler)->void
    {
        std::unique_ptr<void, std::function<void(void*)>> finally(reinterpret_cast<void*>(1), [&](void*)
        {   // called when leaving the scope, also after an exception
            process_finished_handler.on_finished(process_exe, test_success);
        });
        EXPECT_COMP(os, count, run, == , test_run, );
        EXPECT_COMP(os, count, id, == , 1, );
    });
    depgraph_exe__process_done = [&](cppmake::process_exe_t* process, bool success)->void
    {
        EXPECT_COMP(os, count, process, == , &process_exe, );
        EXPECT_COMP(os, count, success, == , test_success, );
    };
    policy__finish = [&](bool& rerun)->bool
    {
        rerun = false;
        return /*finished_successfully=*/true;
    };
    go();
}

/******************************************************************************
build and abort
******************************************************************************/
void build_and_abort(std::ostream& os, int& count)
{
    os << "- " << __func__ << std::endl;
    stb::filesystem::path group("test_root/projects/cppmake");
    stub_depgraph_exe_t* pstub_depgraph_exe = nullptr;
    stub_policy_t* pstub_policy = nullptr;
    stub_process_exe_t process_exe(os, count);
    int const test_run = 123;
    bool const test_success = true;
    std::mutex mt_process_is_running;
    std::condition_variable cv_process_is_running;
    bool process_is_running = false;
    std::mutex mt_process_is_aborting;
    std::condition_variable cv_process_is_aborting;
    bool process_is_aborting = false;
    auto go = [&]() -> void
    {
        std::thread abort_thread([&]()
        {
            {
                std::unique_lock<std::mutex> lock(mt_process_is_running);
                cv_process_is_running.wait(lock, [&] {return process_is_running; });
            }
            abort_execution();
        });
        int ret = execute(0, 0, group, 0);
        assert_all_empty(os, count);
        abort_thread.join();
    };
    std::function<create_empty_dependency_graph_exe_t> create_empty_dependency_graph_exe;
    std::function<stub_depgraph_exe_t::get_next_process_to_start_t> depgraph_exe__get_next_process_to_start;
    std::function<stub_depgraph_exe_t::process_done_t> depgraph_exe__process_done;
    std::function<stub_depgraph_exe_t::run_t> depgraph_exe__run;
    std::function<stub_policy_t::finish_t> policy__finish;
    std::function<create_policy_t> create_policy;
    create_dependency_graph_exe_and_policy_2(
        os,
        count,
        depgraph_exe__get_next_process_to_start,
        depgraph_exe__process_done,
        depgraph_exe__run,
        policy__finish,
        create_empty_dependency_graph_exe,
        create_policy,
        pstub_depgraph_exe,
        pstub_policy);
    depgraph_exe__get_next_process_to_start = [&]()->cppmake::process_exe_t*
    {
        pstub_depgraph_exe->m_get_next_process_to_start.push([&]()->cppmake::process_exe_t*
        {
            return nullptr;
        });
        return &process_exe;
    };
    depgraph_exe__run = [&]()->int
    {
        return test_run;
    };
    process_exe.m_execute.push([&](int run, int id, cppmake::process_finished_handler_t& process_finished_handler)->void
    {
        std::unique_ptr<void, std::function<void(void*)>> finally(reinterpret_cast<void*>(1), [&](void*)
        {   // called when leaving the scope, also after an exception
            process_finished_handler.on_finished(process_exe, test_success);
        });
        EXPECT_COMP(os, count, run, == , test_run, );
        EXPECT_COMP(os, count, id, == , 1, );
        {
            std::lock_guard<std::mutex> lock(mt_process_is_running);
            process_is_running = true;
        }
        cv_process_is_running.notify_one();
        {
            std::unique_lock<std::mutex> lock(mt_process_is_aborting);
            cv_process_is_aborting.wait(lock, [&] {return process_is_aborting; });
        }
    });
    depgraph_exe__process_done = [&](cppmake::process_exe_t* process, bool success)->void
    {
        EXPECT_COMP(os, count, process, == , &process_exe, );
        EXPECT_COMP(os, count, success, == , test_success, );
    };
    process_exe.m_abort.push([&]() 
    {
        {
            std::lock_guard<std::mutex> lock(mt_process_is_aborting);
            process_is_aborting = true;
        }
        cv_process_is_aborting.notify_one();
    });
    policy__finish = [&](bool& rerun)->bool
    {
        rerun = false;
        return /*finished_successfully=*/true;
    };
    go();
}

/******************************************************************************
exception on main thread
******************************************************************************/
void exception_on_main_thread(std::ostream& os, int& count, bool standard, bool early)
{
    os << "- " << __func__ << std::endl;
    stb::filesystem::path group("test_root/projects/cppmake");
    stub_depgraph_exe_t* pstub_depgraph_exe = nullptr;
    stub_policy_t* pstub_policy = nullptr;
    stub_process_exe_t process_exe(os, count);
    std::runtime_error test_exception("test");
    bool const test_success = true;
    auto go = [&]() -> void
    {
        std::stringstream ss;
        std::streambuf* old_buf = std::cout.rdbuf(ss.rdbuf());
        int ret = execute(0, 0, group, 0);
        EXPECT_COMP(os, count, ret, == , -1, << " standard=" << standard << " early=" << early);
        std::cout.rdbuf(old_buf);
        if (!early)
        {
            ASSERT_COMP(ss.str(), == , "", << " standard=" << standard << " early=" << early);
        }
        else if (standard)
        {
            ASSERT_COMP(ss.str(), == , std::string("Early exception: ") + test_exception.what() + '\n', << " standard=" << standard << " early=" << early);
        }
        else
        {
            ASSERT_COMP(ss.str(), == , "Early exception: Non std::exception\n", << " standard=" << standard << " early=" << early);
        }
        assert_all_empty(os, count);
    };
    std::function<create_empty_dependency_graph_exe_t> create_empty_dependency_graph_exe;
    std::function<stub_depgraph_exe_t::get_next_process_to_start_t> depgraph_exe__get_next_process_to_start;
    std::function<stub_depgraph_exe_t::process_done_t> depgraph_exe__process_done;
    std::function<stub_depgraph_exe_t::run_t> depgraph_exe__run;
    std::function<stub_policy_t::finish_t> policy__finish;
    std::function<create_policy_t> create_policy;
    create_dependency_graph_exe_and_policy_2(
        os,
        count,
        depgraph_exe__get_next_process_to_start,
        depgraph_exe__process_done,
        depgraph_exe__run,
        policy__finish,
        create_empty_dependency_graph_exe,
        create_policy,
        pstub_depgraph_exe,
        pstub_policy);
    depgraph_exe__get_next_process_to_start = [&]()->cppmake::process_exe_t*
    {
        pstub_depgraph_exe->m_get_next_process_to_start.push([&]()->cppmake::process_exe_t*
        {
            return nullptr;
        });
        return &process_exe;
    };
    depgraph_exe__run = [&]()->int
    {
        return 0;
    };
    process_exe.m_execute.push([&](int run, int id, cppmake::process_finished_handler_t& process_finished_handler)->void
    {
        std::unique_ptr<void, std::function<void(void*)>> finally(reinterpret_cast<void*>(1), [&](void*)
        {   // called when leaving the scope, also after an exception
            process_finished_handler.on_finished(process_exe, test_success);
        });
        EXPECT_COMP(os, count, id, == , 1, << " standard=" << standard << " early=" << early);
    });
    depgraph_exe__process_done = [&](cppmake::process_exe_t* process, bool success)->void
    {
        EXPECT_COMP(os, count, process, == , &process_exe, << " standard=" << standard << " early=" << early);
        EXPECT_COMP(os, count, success, == , test_success, << " standard=" << standard << " early=" << early);
        if (standard)
        {
            throw test_exception;
        }
        else
        {
            throw 1;
        }
    };
    g_handle_exception.push([&](cppmake::policy_t* ppolicy, std::exception& exc)->bool 
    {
        if (standard)
        {
            EXPECT_COMP(os, count, std::string(exc.what()), == , std::string(test_exception.what()), );
        }
        else
        {
            EXPECT_COMP(os, count, std::string(exc.what()), == , "Non std::exception", << " standard=" << standard << " early=" << early);
        }
        return !early;
    });
    go();
}

/******************************************************************************
exception on process thread
******************************************************************************/
void exception_on_process_thread(std::ostream& os, int& count, bool standard)
{
    os << "- " << __func__ << std::endl;
    stb::filesystem::path group("test_root/projects/cppmake");
    stub_depgraph_exe_t* pstub_depgraph_exe = nullptr;
    stub_policy_t* pstub_policy = nullptr;
    stub_process_exe_t process_exe(os, count);
    std::runtime_error test_exception("test");
    bool const test_success = true;
    auto go = [&]() -> void
    {
        std::stringstream ss;
        std::streambuf* old_buf = std::cout.rdbuf(ss.rdbuf());
        int ret = execute(0, 0, group, 0);
        EXPECT_COMP(os, count, ret, == , -1, << " standard=" << standard);
        std::cout.rdbuf(old_buf);
        ASSERT_COMP(ss.str(), == , "", << " standard=" << standard);
        assert_all_empty(os, count);
    };
    std::function<create_empty_dependency_graph_exe_t> create_empty_dependency_graph_exe;
    std::function<stub_depgraph_exe_t::get_next_process_to_start_t> depgraph_exe__get_next_process_to_start;
    std::function<stub_depgraph_exe_t::process_done_t> depgraph_exe__process_done;
    std::function<stub_depgraph_exe_t::run_t> depgraph_exe__run;
    std::function<stub_policy_t::finish_t> policy__finish;
    std::function<create_policy_t> create_policy;
    create_dependency_graph_exe_and_policy_2(
        os,
        count,
        depgraph_exe__get_next_process_to_start,
        depgraph_exe__process_done,
        depgraph_exe__run,
        policy__finish,
        create_empty_dependency_graph_exe,
        create_policy,
        pstub_depgraph_exe,
        pstub_policy);
    depgraph_exe__get_next_process_to_start = [&]()->cppmake::process_exe_t*
    {
        pstub_depgraph_exe->m_get_next_process_to_start.push([&]()->cppmake::process_exe_t*
        {
            return nullptr;
        });
        return &process_exe;
    };
    depgraph_exe__run = [&]()->int
    {
        return 0;
    };
    process_exe.m_execute.push([&](int run, int id, cppmake::process_finished_handler_t& process_finished_handler)->void
    {
        std::unique_ptr<void, std::function<void(void*)>> finally(reinterpret_cast<void*>(1), [&](void*)
        {   // called when leaving the scope, also after an exception
            process_finished_handler.on_finished(process_exe, test_success);
        });
        EXPECT_COMP(os, count, id, == , 1, << " standard=" << standard);
        if (standard)
        {
            throw test_exception;
        }
        else
        {
            throw 1;
        }
    });
    depgraph_exe__process_done = [&](cppmake::process_exe_t* process, bool success)->void
    {
        EXPECT_COMP(os, count, process, == , &process_exe, << " standard=" << standard);
        EXPECT_COMP(os, count, success, == , test_success, << " standard=" << standard);
    };
    g_handle_exception.push([&](cppmake::policy_t* ppolicy, std::exception& exc)->bool
    {
        if (standard)
        {
            EXPECT_COMP(os, count, std::string(exc.what()), == , std::string(test_exception.what()), );
        }
        else
        {
            EXPECT_COMP(os, count, std::string(exc.what()), == , "Non std::exception", << " standard=" << standard);
        }
        return true;
    });
    go();
}

__declspec(dllexport) void cppmake_test_executor(std::ostream& os, int& count)
{
    os << "begin " << __func__ << std::endl;
    nothing_to_build(os, count, /*finished_successfully=*/false, /*rerun=*/false, /*expect_return=*/-1);
    nothing_to_build(os, count, /*finished_successfully=*/false, /*rerun=*/true, /*expect_return=*/-1);
    nothing_to_build(os, count, /*finished_successfully=*/true, /*rerun=*/false, /*expect_return=*/0);
    nothing_to_build(os, count, /*finished_successfully=*/true, /*rerun=*/true, /*expect_return=*/1);
    print_line_on_cout(os, count);
    do_build_but_no_work_to_do(os, count);
    do_build_and_do_process(os, count);
    build_and_abort(os, count);
    exception_on_main_thread(os, count, /*standard=*/false, /*early=*/false);
    exception_on_main_thread(os, count, /*standard=*/false, /*early=*/true);
    exception_on_main_thread(os, count, /*standard=*/true, /*early=*/false);
    exception_on_main_thread(os, count, /*standard=*/true, /*early=*/true);
    exception_on_process_thread(os, count, /*standard=*/false);
    exception_on_process_thread(os, count, /*standard=*/true);
    os << "end   " << __func__ << std::endl;
}

/* To do
- thread(), id() member functions niet mocken, check alle mocks hierop
- naamgeving mock ipv stub
- check goed de volgorde van executie
- maak sequence diagrammen
- check goed wat er nu getest moet worden; geen onnodige duplicatie
- zijn er 2 helper functies nodig???
- simuleer policy__finish beter, true/false
*/
