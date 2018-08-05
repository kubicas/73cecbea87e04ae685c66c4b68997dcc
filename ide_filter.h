#ifndef CPPMAKE__IDE_FILTER_EXE_H
#define CPPMAKE__IDE_FILTER_EXE_H

#include "logger.h"
#include <memory>
#include <string>

namespace cppmake {

enum class ide_t
{
    unknown,
    visual_studio,
    csv,
};


class ide_filter_t
{
public:
    virtual ~ide_filter_t() = 0;
    virtual std::string to_string(int run, int id, stb::filesystem::path const& path, int line, int column, msg_t msg, std::string const& msg_id, std::string const& text) = 0;
    virtual std::string to_string(int run, int id, msg_t msg, std::string const& msg_id, std::string const& text) = 0;
    virtual std::string to_string(int run, int id, stb::filesystem::path const& path, int line, int column, msg_t msg, std::string const& text) = 0;
    virtual std::string to_string(int run, int id, msg_t msg, std::string const& text) = 0;
};
inline ide_filter_t::~ide_filter_t() {}

std::unique_ptr<ide_filter_t> create_ide_filter(ide_t ide);

}; // namespace cppmake

#endif // CPPMAKE__IDE_FILTER_EXE_H
