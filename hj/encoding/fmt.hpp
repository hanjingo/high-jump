#ifndef FMT_HPP
#define FMT_HPP

#include <fmt/core.h>

namespace hj
{

template <typename... Args>
std::string fmt(const char* style, Args&&... args)
{
    return fmt::vformat(style, fmt::make_format_args(std::forward<Args>(args)...));
}

} // namespace hj

#endif // FMT_HPP