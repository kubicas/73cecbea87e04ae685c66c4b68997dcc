/*******************************
* Responsibilities of main.cpp *
********************************
1) Copy the makefile.dll to the procts directory
2) Assure that only one cppmake.exe is running within projects
3) Dynamically link to two functions of makefile.dll:
   - execute()
   - abort_execution()
4) Call execute() while passing the commandline parameters 
5) In case of a control-C, call abort_execution
6) In case execute() returns 1, start all over from 1)
   This allows makefile.dll to build itself and after being called again it builds the targets
7) Print "Build succeeded" when the build was succesfull and "Build failed" otherwise
8) Return 0 when the build was succesfull and -1 otherwise

Implementation of cppmake.exe is (unfortunatelly) platform dependent;
For the user, cppmake behaves identical on any platform (just like git and LaTeX)
Implementation of makefile.dll is intended to be as much standard C++ as possible.
*/

#include <iostream>
#include "stb/thread.h" // stubbable version of <thread>
#include "stb/filesystem.h" // stubbable version of <filesystem>
#include "exception/exception.h"

#include <windows.h>

#include "cppmake/executor.h"

namespace // anonymous
{

bool done = false;
cppmake::abort_execution_t abort_execution = 0;
cppmake::execute_t execute = 0;

BOOL ctrl_handler(DWORD fdwCtrlType)
{
    if (abort_execution)
    {
        std::cout << "Aborting ";
        abort_execution();
        while (!done)
        {
            stb::this_thread::sleep_for(std::chrono::milliseconds(250));
            std::cout << '.';
        }
        std::cout << std::endl;
        return TRUE;
    }
    else
    {
        std::cout << "cppmake executor exception: Gracefull aborting failed" << std::endl;
        return FALSE;
    }
}

#define PROCTS "procts"

void determine_source_target_and_group(stb::filesystem::path& source, stb::filesystem::path& target, stb::filesystem::path& group)
{
    stb::filesystem::path cwd = stb::filesystem::current_path();
    if (*--cwd.end() == PROCTS)
    {
        THROW(std::runtime_error, << "Current working directory must be a subdirectory of '" PROCTS "'");
    }
    stb::filesystem::path::const_iterator it(cwd.end());
    while(true)
    {
        if (it == cwd.begin())
        {
            THROW(std::runtime_error, << "Current working directory is not in '" PROCTS "'");
        }
        --it;
        if (!it->compare(PROCTS))
        {
            ++it;
            break;
        }
    }
    std::for_each(cwd.begin(), ++it, [&group](stb::filesystem::path const& path) { group /= path; });
    source = group;
    source /= "tgt";
    source /= TARGET_DIR;
    source /= "makefile.dll";
    if (!exists(source))
    {
        THROW(std::runtime_error, << "'makefile.dll' does not exist in 'tgt/" TARGET_DIR "/");
    }
    target = group;
    target /= "makefile.dll";
}

}; // namespace anonymous

int main(int argc, char* argv[])
{
    int ret = -1;
    stb::filesystem::path target;
    HINSTANCE dlso_handle = nullptr;
    bool remove_target = false;
    try
    {
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE);
        stb::filesystem::path source;
        stb::filesystem::path group;
        determine_source_target_and_group(source, target, group);
        int run_count = 0;
        while (true)
        {
            std::error_code ec;
            if (!stb::filesystem::copy_file(source, target, ec))
            {
                THROW(std::runtime_error, << "'.../" PROCTS "/makefile.dll' exists already.\nIs another make running in projects?\nWas a former make hard aborted?");
            }
            remove_target = true;
            UINT old_error_mode = SetErrorMode(SEM_FAILCRITICALERRORS);
            SetErrorMode(old_error_mode | SEM_FAILCRITICALERRORS);
            dlso_handle = LoadLibraryA(target.u8string().c_str());
            if (!dlso_handle)
            {
                THROW(std::runtime_error, << "Cannot load '.../" PROCTS "/makefile.dll'");
            }
            SetErrorMode(old_error_mode);
            abort_execution = (cppmake::abort_execution_t)GetProcAddress(dlso_handle, "abort_execution");
            execute = (cppmake::execute_t)GetProcAddress(dlso_handle, "execute");
            if ((!abort_execution) || (!execute))
            {
                THROW(std::runtime_error, << "Cannot link to functions in makefile.dll");
            }
            ret = execute(argc, const_cast<char const**>(argv), group, run_count++);
            if (ret != 1)
            {
                break;
            }
            abort_execution = nullptr;
            FreeLibrary(dlso_handle);
            dlso_handle = nullptr;
        }
    }
    catch (std::exception& e)
    {
        std::cout << "cppmake executor exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "cppmake executor non-std::exception." << std::endl;
    }
    abort_execution = nullptr;
    FreeLibrary(dlso_handle);
    if (remove_target)
    {
        stb::filesystem::remove(target);
    }
    done = true;
    if (ret)
    {
        std::cout << "Build failed" << std::endl;
    }
    else
    {
        std::cout << "Build succeeded" << std::endl;
    }
    return ret;
}