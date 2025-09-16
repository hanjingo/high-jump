#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <vector>

enum class fmt_target
{
    memory,
    file
};

class formator
{
public:
    static inline std::vector<std::string> supported_styles()
    {
        return std::vector<std::string>{"hex", "base64", "none", ""};
    }

    static inline bool is_supported_style(const std::string& fmt)
    {
        return fmt == "hex" || fmt == "base64" || fmt == "none" || fmt == "";
    }

    static int format(
        std::string& out,
        const std::string& in,
        const std::string& fmt,
        const fmt_target tgt);

    static int unformat(
        std::string& out,
        const std::string& in,
        const std::string& fmt,
        const fmt_target tgt);

private:
    static int _format_memory(
        std::string& out,
        const std::string& in,
        const std::string& fmt);

    static int _unformat_memory(
        std::string& out,
        const std::string& in,
        const std::string& fmt);

    static int _format_file(
        const std::string& out,
        const std::string& in,
        const std::string& fmt);

    static int _unformat_file(
        const std::string& out,
        const std::string& in,
        const std::string& fmt);
};

#endif