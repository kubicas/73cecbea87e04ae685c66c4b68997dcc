#ifndef CPPMAKE__LOGGER_H
#define CPPMAKE__LOGGER_H

#include <string>
#include "stb/filesystem.h" // stubbable version of <filesystem>

namespace cppmake {

enum class msg_t : int8_t
{
    unknown,
    error,
    warning,
    note,
};

class process_config_t;

class logger_t
{
public:
    virtual ~logger_t() = 0;
    virtual void on_log_line(process_config_t const& process, stb::filesystem::path const& path, int line, int column, msg_t msg, std::string const& msg_id, std::string const& text) = 0;
    virtual void on_log_line(process_config_t const& process, msg_t msg, std::string const& msg_id, std::string const& text) = 0;
    virtual void on_log_line(process_config_t const& process, stb::filesystem::path const& path, int line, int column, msg_t msg, std::string const& text) = 0;
    virtual void on_log_line(process_config_t const& process, msg_t msg, std::string const& text) = 0;
};
inline logger_t::~logger_t() {}

}; // namespace cppmake


#endif // CPPMAKE__LOGGER_H
