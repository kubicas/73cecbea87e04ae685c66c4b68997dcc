#ifndef CPPMAKE__PATH_H
#define CPPMAKE__PATH_H

#include <memory>
#include "stb/filesystem.h" // stubbable version of <filesystem>

namespace cppmake {

class path_t
{
public:
    virtual ~path_t() = 0;
};

inline path_t::~path_t() {}

std::unique_ptr<path_t> create_path(stb::filesystem::path const& path);


}; // namespace cppmake

#endif // CPPMAKE__PATH_H
