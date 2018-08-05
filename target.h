#ifndef CPPMAKE__TARGET_H
#define CPPMAKE__TARGET_H

#include <string>

namespace cppmake {

enum class platform_t : int16_t
{
    unknown,
    winx86,
    winx64,
    linux,
    osx
};

enum class toolset_t : int16_t
{
    unknown,
    ms,
    clang,
    gcc
};

enum class config_t : int16_t
{
    unknown,
    release,
    debug,
};

class target_t
{
public:
    virtual ~target_t() = 0;
    virtual std::string name() = 0;
    virtual platform_t host_platform() = 0;
    virtual toolset_t toolset() = 0;
    virtual platform_t tgt_platform() = 0;
    virtual config_t config() = 0;
    virtual bool& build() = 0;
};
inline target_t::~target_t() {}

}; // namespace cppmake

#endif // CPPMAKE__TARGET_H
