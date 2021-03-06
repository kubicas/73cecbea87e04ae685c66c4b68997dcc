/***********************************
* Responsibilities of executor.cpp *
************************************
RT.executor.create.policy
    Create a the policy and pass the commandline arguements
RT.executor.build
    In case a build needs to be performed:
    start the processes which are supplied by the dependency graph
    report them complete when a process is finished
    continue doing this until no more processes are supplied
RT.executor.build.threading
    Handle all threading threading issues. This includes
    presenting a thread safe print line so that policy can print on the console
RT.executor.build.abort
    Handle user abort by aborting all running processes. Finish book keeping in a controlled way.
RT.executor.exceptions
    Catching all exceptions and pass them on to policy when possible
RT.executor.return
    Return
        0 when build finished succefully and no rerun is needed, 
        1 when build finished succefully and rerun is needed,
       -1 when the build was not successfull
 */

#include "cppmake/executor.h"
#include "depgraph_exe.h"
#include "process_exe.h"
#include "policy.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <iostream>
#include <string>
#include <set>
#include <unordered_map>
#include "stb/future.h"
#ifdef TEST
#include "test/test.h"
#endif // TEST

namespace // anonymous
{

struct thread_safe_console_t
    : cppmake::thread_safe_console_t
{
    explicit thread_safe_console_t(std::ostream& os)
        : m_os(os)
    {}
    virtual void print_line(std::string const& text) override
    {
        std::lock_guard<std::mutex> os_lock_guard(m_mutex);
        m_os << text << std::endl;
    }
    std::ostream& m_os;
    std::mutex m_mutex;
};

struct executor_t
    : cppmake::process_finished_handler_t
{
    explicit executor_t(cppmake::depgraph_exe_t& depgraph)
        : m_depgraph(depgraph)
        , m_abort(false)
        , m_aborting(false)
        , m_pprocess_finished(nullptr)
        , m_process_success(false)
        , m_process_id(0)
    {}

    void execute_graph()
    {
        bool work_to_do = start_some_processes();
        while (work_to_do)
        {
            wait_for_event();
            work_to_do = start_some_processes();
        }
    }

    void abort()
    {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_abort = true;
            m_condition_variable.notify_one();
        }
    }

    bool start_some_processes()
    {
        std::lock_guard<std::mutex> os_lock_guard(m_mutex);
        while (
            (m_running_processes.size() < m_processor_count) &&
            !m_aborting)
        {
            cppmake::process_exe_t* pprocess = m_depgraph.get_next_process_to_start();
            if (!pprocess)
            {
                break;
            }
            m_running_processes[++m_process_id] = pprocess;
            pprocess->future() = std::async(stb::launch::async, &cppmake::process_exe_t::execute, pprocess,
                m_depgraph.run(),
                m_process_id,
                std::ref(*static_cast<process_finished_handler_t*>(this)));
        }
        return !m_running_processes.empty(); // = work_to_do
    }

    void wait_for_event()
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_condition_variable.wait(lk, [this]()
        {
            return (
                m_abort ||
                m_pprocess_finished);
        });
        if (m_pprocess_finished)
        {
            m_depgraph.process_done(m_pprocess_finished, m_process_success);
            m_running_processes.erase(m_pprocess_finished->id());
            m_pprocess_finished->future().get();
            m_pprocess_finished = nullptr;
        }
        if (m_abort)
        {
            m_abort = false;
            m_aborting = true;
            for (std::pair<int, cppmake::process_exe_t*> it : m_running_processes)
            {
                it.second->abort();
            }
        }
        if (m_aborting)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void on_finished(cppmake::process_exe_t& process, bool success) override
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_pprocess_finished = &process;
        m_process_success = success;
        m_condition_variable.notify_one();
    }

    std::mutex m_mutex;
    std::condition_variable m_condition_variable;
    cppmake::depgraph_exe_t& m_depgraph;
    std::unordered_map<int, cppmake::process_exe_t*> m_running_processes;
    bool m_abort;
    bool m_aborting;
    cppmake::process_exe_t* m_pprocess_finished;
    bool m_process_success;
    int m_process_id;
    size_t const m_processor_count = std::thread::hardware_concurrency();
};

std::atomic<executor_t*> pexecutor = nullptr;

void handle_any_exception(cppmake::policy_t* ppolicy, std::exception& exc)
{
    if (!handle_exception(ppolicy, exc))
    {
        std::cout << "Early exception: " << exc.what() << std::endl;
    }
}

}; // namespace anonymous

#ifndef TEST
extern "C" __declspec(dllexport)
#endif // TEST
int execute(int argc, char const* argv[], stb::filesystem::path const& group, int run_count)
{
    bool ok = true;
    bool rerun = false;
    std::unique_ptr<cppmake::policy_t> ppolicy;
    try
    {
        std::set<std::string> arguments;
        if (argc > 0)
        {
            arguments.insert(argv + 1, argv + argc);
            arguments.insert(std::string("program_name=") + argv[0]);
        }
        std::unique_ptr<cppmake::depgraph_exe_t> pdepgraph = cppmake::create_empty_dependency_graph_exe();
        ::thread_safe_console_t thread_safe_console(std::cout);
        ppolicy = cppmake::create_policy(cast(*pdepgraph), arguments, thread_safe_console);
        if (ppolicy->build())
        {
            executor_t executor(*pdepgraph);
            pexecutor = &executor; // to allow aborts
            executor.execute_graph();
            ok &= !executor.m_aborting;
        }
        ok &= ppolicy->finish(rerun);
        ppolicy.reset();
    }
#ifdef TEST
    catch (test::error_t&)
    {
        throw;
    }
#endif // TEST
    catch (std::exception& exc)
    {
        ok = false;
        handle_any_exception(ppolicy.get(), exc);
    }
    catch (...)
    {
        ok = false;
        std::runtime_error non_std_exception("Non std::exception");
        handle_any_exception(ppolicy.get(), non_std_exception);
    }
    return ok ? (rerun ? 1 : 0) : -1;
}

#ifndef TEST
extern "C" __declspec(dllexport)
#endif // TEST
void abort_execution()
{
    if (pexecutor)
    {
        pexecutor.load()->abort();
    }
}

