#include "ide_filter.h"

#include <sstream>
#include "stb/filesystem.h" // stubbable version of <filesystem>

namespace // anonymous
{

struct visual_studio_ide_filter_t
    : cppmake::ide_filter_t
{
    visual_studio_ide_filter_t()
    {}
    std::string to_string(int run, int id, stb::filesystem::path const& path, int line, int column, cppmake::msg_t msg, std::string const& msg_id, std::string const& text) override
    {
        std::stringstream ss;
        ss << id << ' ' << path << '(' << line << "): " << text;
        return ss.str();
    }
    std::string to_string(int run, int id, cppmake::msg_t msg, std::string const& msg_id, std::string const& text) override
    {
        std::stringstream ss;
        ss << id << ' ' << text;
        return ss.str();
    }
    std::string to_string(int run, int id, stb::filesystem::path const& path, int line, int column, cppmake::msg_t msg, std::string const& text) override
    {
        std::stringstream ss;
        ss << id << ' ' << path << '(' << line << "): " << text;
        return ss.str();
    }
    std::string to_string(int run, int id, cppmake::msg_t msg, std::string const& text) override
    {
        std::stringstream ss;
        ss << id << ' ' << text;
        return ss.str();
    }
};


};

namespace cppmake
{

std::unique_ptr<ide_filter_t> create_ide_filter(ide_t ide)
{
    switch (ide)
    {
    case ide_t::visual_studio:
    default:
        return std::make_unique<::visual_studio_ide_filter_t>();
    }
}

}; // namespace cppmake
