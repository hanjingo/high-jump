#ifndef I18N_HPP
#define I18N_HPP

#include <unicode/unistr.h>
#include <unicode/locid.h>
#include <unicode/numfmt.h>
#include <unicode/datefmt.h>
#include <unicode/msgfmt.h>
#include <unicode/coll.h>
#include <unicode/translit.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/uloc.h>
#include <unicode/resbund.h>
#include <unicode/ures.h>

#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <thread>
#include <atomic>

namespace libcpp 
{

class i18n 
{
public:
    class translator 
    {
    private:
        std::unordered_map<std::string, std::string> translations_;
        std::string locale_name_;

    public:
        translator() = default;
        explicit translator(const std::string& locale) : locale_name_(locale) {}

        inline const std::string& locale() const { return locale_name_; }
        inline void set_locale(const std::string& locale) { locale_name_ = locale; }
        inline bool is_empty() const { return translations_.empty(); }
        inline void clear() { translations_.clear(); }
        inline void add(const std::string& key, const std::string& value) { translations_[key] = value; }

        bool load_from_map(const std::unordered_map<std::string, std::string>& translations) 
        {
            translations_ = translations;
            return true;
        }

        std::string translate(const std::string& key) const 
        {
            auto it = translations_.find(key);
            if (it != translations_.end())
                return it->second;

            return key;
        }

        template<typename... Args>
        std::string translate(const std::string& key, Args... args) const 
        {
            std::string pattern = translate(key);
            if (pattern.empty()) 
                return key;
            
            return _format_message(pattern, args...);
        }

        bool load_from_properties(const std::string& file_path) 
        {
            std::ifstream file(file_path);
            if (!file.is_open())
                return false;

            std::string line;
            size_t eq_pos;
            while (std::getline(file, line)) 
            {
                if (line.empty() || line[0] == '#' || line[0] == '!')
                    continue;

                eq_pos = line.find('=');
                if (eq_pos == std::string::npos) 
                    continue;

                std::string key = _trim(line.substr(0, eq_pos));
                std::string value = _trim(line.substr(eq_pos + 1));
                value = _unescape_unicode(value);
                translations_[key] = value;
            }
            file.close();
            return true;
        }

        bool load_from_resource_bundle(const std::string& bundle_name, 
                                       const std::string& locale_name = "") 
        {
            UErrorCode status = U_ZERO_ERROR;
            std::string loc = locale_name.empty() ? locale_name_ : locale_name;
            icu::Locale locale(loc.c_str());
            icu::ResourceBundle bundle(bundle_name.c_str(), locale, status);
            if (U_FAILURE(status))
                return false;

            return _load_from_resource_bundle_recursive(bundle, "");
        }

        bool save_to_properties(const std::string& file_path) const 
        {
            std::ofstream file(file_path);
            if (!file.is_open()) 
                return false;

            file << "# Translation file for locale: " << locale_name_ << "\n";
            file << "# Generated on " << _current_timestamp() << "\n\n";
            for (const auto& [key, value] : translations_) 
                file << key << "=" << _escape_unicode(value) << "\n";

            file.close();
            return true;
        }

    private:
        template<typename T>
        void _add_args(std::vector<icu::Formattable>& formattables, T&& arg) const 
        {
            if constexpr (std::is_arithmetic_v<std::decay_t<T>>) 
            {
                formattables.emplace_back(static_cast<double>(arg));
                return;
            }

            icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(std::string(arg));
            formattables.emplace_back(unicode_str);
        }

        template<typename T, typename... Rest>
        void _add_args(std::vector<icu::Formattable>& formattables, T&& first, Rest&&... rest) const 
        {
            _add_args(formattables, std::forward<T>(first));
            _add_args(formattables, std::forward<Rest>(rest)...);
        }

        template<typename... Args>
        std::string _format_message(const std::string& pattern, Args... args) const 
        {
            UErrorCode status = U_ZERO_ERROR;
            icu::Locale loc(locale_name_.empty() ? "en_US" : locale_name_.c_str());
            icu::UnicodeString unicode_pattern = icu::UnicodeString::fromUTF8(pattern);
            icu::MessageFormat formatter(unicode_pattern, loc, status);
            if (U_FAILURE(status))
                return pattern;
            
            std::vector<icu::Formattable> formattables;
            _add_args(formattables, args...);
            icu::UnicodeString result;
            icu::FieldPosition field_pos(0);
            formatter.format(formattables.data(), 
                             static_cast<int32_t>(formattables.size()), 
                             result, 
                             field_pos,
                             status);
            if (U_SUCCESS(status)) 
            {
                std::string utf8_result;
                result.toUTF8String(utf8_result);
                return utf8_result;
            }

            return pattern;
        }

        bool _load_from_resource_bundle_recursive(const icu::ResourceBundle& bundle, 
                                                 const std::string& prefix) 
        {
            UErrorCode status = U_ZERO_ERROR;
            int32_t size = bundle.getSize();
            for (int32_t i = 0; i < size; ++i) 
            {
                icu::ResourceBundle child = bundle.get(i, status);
                if (U_FAILURE(status)) 
                    continue;

                std::string key = prefix.empty() ? child.getKey() : prefix + "." + child.getKey();
                if (child.getType() == URES_STRING) 
                {
                    icu::UnicodeString value = child.getString(status);
                    if (!U_SUCCESS(status)) 
                        continue;

                    std::string utf8_value;
                    value.toUTF8String(utf8_value);
                    translations_[key] = utf8_value;
                } 
                else if (child.getType() == URES_TABLE) 
                {
                    _load_from_resource_bundle_recursive(child, key);
                }
            }
            return true;
        }

        std::string _trim(const std::string& str) const 
        {
            size_t start = str.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) 
                return "";

            size_t end = str.find_last_not_of(" \t\r\n");
            return str.substr(start, end - start + 1);
        }

        std::string _unescape_unicode(const std::string& str) const 
        {
            std::string result;
            std::string hex;
            std::string hex2;
            uint16_t code;
            uint16_t code2;
            for (size_t i = 0; i < str.length(); ++i) 
            {
                if (str[i] == '\\' && i + 5 < str.length() && str[i + 1] == 'u') 
                {
                    hex = str.substr(i + 2, 4);
                    code = static_cast<uint16_t>(std::stoul(hex, nullptr, 16));
                    if (U16_IS_LEAD(code) && i + 11 < str.length() && str[i + 6] == '\\' && str[i + 7] == 'u') 
                    {
                        hex2 = str.substr(i + 8, 4);
                        code2 = static_cast<uint16_t>(std::stoul(hex2, nullptr, 16));
                        if (!U16_IS_TRAIL(code2)) 
                            continue;

                        UChar32 full_char = U16_GET_SUPPLEMENTARY(code, code2);
                        icu::UnicodeString unicode_char(full_char);
                        std::string utf8_char;
                        unicode_char.toUTF8String(utf8_char);
                        result += utf8_char;
                        i += 11; // skip \uXXXX
                        continue;
                    }
                    
                    icu::UnicodeString unicode_char(code);
                    std::string utf8_char;
                    unicode_char.toUTF8String(utf8_char);
                    result += utf8_char;
                    i += 5; // skip \uXXXX
                    continue;
                } 
                else 
                {
                    result += str[i];
                }
            }
            return result;
        }

        std::string _escape_unicode(const std::string& str) const 
        {
            icu::UnicodeString unicode_str = icu::UnicodeString::fromUTF8(str);
            std::string result;
            for (int32_t i = 0; i < unicode_str.length(); ++i) 
            {
                UChar32 ch = unicode_str.char32At(i);
                if (ch < 128 && ch > 31 && ch != '\\' && ch != '=') 
                {
                    result += static_cast<char>(ch);
                } 
                else 
                {
                    if (ch <= 0xFFFF) 
                    {
                        std::ostringstream oss;
                        oss << "\\u" << std::hex << std::setfill('0') << std::setw(4) << ch;
                        result += oss.str();
                    } 
                    else 
                    {
                        UChar lead = U16_LEAD(ch);
                        UChar trail = U16_TRAIL(ch);
                        std::ostringstream oss;
                        oss << "\\u" << std::hex << std::setfill('0') << std::setw(4) << lead
                            << "\\u" << std::hex << std::setfill('0') << std::setw(4) << trail;
                        result += oss.str();
                    }
                }

                if (U16_IS_LEAD(unicode_str[i]) && i + 1 < unicode_str.length())
                    ++i;
            }
            return result;
        }

        std::string _current_timestamp() const 
        {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return oss.str();
        }
    };

public:
    static i18n& instance() 
    {
        if (!instance_) 
            instance_ = std::make_unique<i18n>();

        return *instance_;
    }

    inline void set_locale(const std::string& locale) { current_locale_ = locale; }
    inline const std::string& locale() const { return current_locale_; }
    inline void remove(const std::string& name) { translators_.erase(name); }
    inline void install(const std::string& name, std::unique_ptr<translator> trans) 
    {
        trans->set_locale(current_locale_);
        translators_[name] = std::move(trans);
    }

    std::string translate(const std::string& key) const 
    {
        auto current_it = translators_.find(current_locale_);
        if (current_it != translators_.end()) 
        {
            std::string result = current_it->second->translate(key);
            if (result != key)
                return result;
        }
        
        auto main_it = translators_.find("main");
        if (main_it != translators_.end() && main_it->first != current_locale_) 
        {
            std::string result = main_it->second->translate(key);
            if (result != key)
                return result;
        }
        
        for (const auto& [name, trans] : translators_) 
        {
            if (name != current_locale_ && name != "main") 
            {
                std::string result = trans->translate(key);
                if (result != key)
                    return result;
            }
        }

        return key;
    }

    template<typename... Args>
    std::string translate(const std::string& key, Args... args) const 
    {
        auto current_it = translators_.find(current_locale_);
        if (current_it != translators_.end()) 
        {
            std::string result = current_it->second->translate(key, args...);
            if (result != key)
                return result;
        }
        
        auto main_it = translators_.find("main");
        if (main_it != translators_.end() && main_it->first != current_locale_) 
        {
            std::string result = main_it->second->translate(key, args...);
            if (result != key)
                return result;
        }
        
        for (const auto& [name, trans] : translators_) 
        {
            if (name != current_locale_ && name != "main") 
            {
                std::string result = trans->translate(key, args...);
                if (result != key)
                    return result;
            }
        }

        return key;
    }

    bool load_translations_from_directory(const std::string& directory_path, 
                                          const std::string& base_name = "translations") 
    {
        if (!std::filesystem::exists(directory_path))
            return false;

        bool loaded_any = false;
        std::string filename;
        size_t locale_start;
        size_t locale_end;
        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) 
        {
            if (!entry.is_regular_file())
                continue;

            filename = entry.path().filename().string();
            if (filename.find(base_name + "_") != 0) 
                continue;

            locale_start = base_name.length() + 1;
            locale_end = filename.find('.', locale_start);
            if (locale_end == std::string::npos) 
                continue;

            std::string locale = filename.substr(locale_start, locale_end - locale_start);
            auto trans = std::make_unique<translator>(locale);
            if (trans->load_from_properties(entry.path().string())) 
            {
                install(locale, std::move(trans));
                loaded_any = true;
            }
        }

        return loaded_any;
    }

    bool load_translation_auto(const std::string& directory_path, 
                               const std::string& base_name = "translations") 
    {
        std::string filename = base_name + "_" + current_locale_;
        std::vector<std::string> extensions = {".properties"};
        for (const auto& ext : extensions) 
        {
            std::string file_path = directory_path + "/" + filename + ext;
            if (!std::filesystem::exists(file_path)) 
                continue;

            auto trans = std::make_unique<translator>(current_locale_);
            bool loaded = false;
            
            if (ext == ".properties")
                loaded = trans->load_from_properties(file_path);
            
            if (loaded) 
            {
                install("main", std::move(trans));
                return true;
            }
        }
        
        return false;
    }

private:
    static std::unique_ptr<i18n> instance_;
    std::string current_locale_;
    std::unordered_map<std::string, std::unique_ptr<translator>> translators_;
};

std::unique_ptr<i18n> i18n::instance_;

inline std::string tr(const std::string& key, const std::string& default_text = "") 
{
    return i18n::instance().translate(key, default_text);
}

template<typename... Args>
inline std::string tr(const std::string& key, Args... args) 
{
    return i18n::instance().translate(key, args...);
}

} // namespace libcpp

#endif // I18N_HPP