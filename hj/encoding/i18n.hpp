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

namespace hj 
{

class i18n 
{
public:
    class translator 
    {
    public:
        translator() = default;
        explicit translator(const std::string& locale) : _locale_name(locale) {}

        inline const std::string& locale() const { return _locale_name; }
        inline void set_locale(const std::string& locale) { _locale_name = locale; }
        inline bool is_empty() const { return _translations.empty(); }
        inline void clear() { _translations.clear(); }
        inline void add(const std::string& key, const std::string& value) { _translations[key] = value; }

        bool load_from_map(const std::unordered_map<std::string, std::string>& translations) 
        {
            _translations = translations;
            return true;
        }

        std::string translate(const std::string& key) const 
        {
            auto it = _translations.find(key);
            if (it != _translations.end())
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
            std::ifstream file(file_path, std::ios::binary);
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
                _translations[key] = value;
            }
            file.close();
            return true;
        }

        bool load_from_resource_bundle(const std::string& bundle_name, 
                                       const std::string& locale_name = "") 
        {
            UErrorCode status = U_ZERO_ERROR;
            std::string loc = locale_name.empty() ? _locale_name : locale_name;
            icu::Locale locale(loc.c_str());
            icu::ResourceBundle bundle(bundle_name.c_str(), locale, status);
            if (U_FAILURE(status))
                return false;

            return _load_from_resource_bundle_recursive(bundle, "");
        }

        bool save_to_properties(const std::string& file_path) const 
        {
            std::ofstream file(file_path, std::ios::binary);
            if (!file.is_open()) 
                return false;

            file << "# Translation file for locale: " << _locale_name << "\n";
            file << "# Generated on " << _current_timestamp() << "\n\n";
            for (const auto& [key, value] : _translations) 
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
            icu::Locale loc(_locale_name.empty() ? "en_US" : _locale_name.c_str());
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
                    _translations[key] = utf8_value;
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
            for (size_t i = 0; i < str.length(); ++i) 
            {
                if (str[i] != '\\') 
                {
                    result += str[i];
                    continue;
                }
                if (i + 1 >= str.length()) 
                {
                    result += str[i];
                    continue;
                }
                if (str[i + 1] == 'n') 
                {
                    result += '\n';
                    ++i;
                } 
                else if (str[i + 1] == 't') 
                {
                    result += '\t';
                    ++i;
                } 
                else if (str[i + 1] == '\\') 
                {
                    result += '\\';
                    ++i;
                } 
                else if (str[i + 1] == 'u' && i + 5 < str.length()) 
                {
                    std::string hex = str.substr(i + 2, 4);
                    uint16_t code = static_cast<uint16_t>(std::stoul(hex, nullptr, 16));
                    icu::UnicodeString unicode_char(code);
                    std::string utf8_char;
                    unicode_char.toUTF8String(utf8_char);
                    result += utf8_char;
                    i += 5;
                }
            }
            return result;
        }

        std::string _escape_unicode(const std::string& str) const 
        {
            std::string result;
            for (size_t i = 0; i < str.size(); ++i) {
                unsigned char ch = str[i];
                switch (ch) 
                {
                    case '\n': result += "\\n"; break;
                    case '\t': result += "\\t"; break;
                    case '\\': result += "\\\\"; break;
                    case '=':  result += "\\="; break;
                    case ':':  result += "\\:"; break;
                    default:
                        result += ch;
                }
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

    private:
        std::unordered_map<std::string, std::string> _translations;
        std::string _locale_name;
    };

public:
    static i18n& instance() 
    {
        static i18n _instance;
        return _instance;
    }

    inline void set_locale(const std::string& locale) { _current_locale = locale; }
    inline const std::string& locale() const { return _current_locale; }
    inline void remove(const std::string& name) { _translators.erase(name); }
    inline void install(const std::string& name, std::unique_ptr<translator> trans) 
    {
        trans->set_locale(_current_locale);
        _translators[name] = std::move(trans);
    }

    std::string translate(const std::string& key) const 
    {
        auto current_it = _translators.find(_current_locale);
        if (current_it != _translators.end()) 
        {
            std::string result = current_it->second->translate(key);
            if (result != key)
                return result;
        }
        
        auto main_it = _translators.find("main");
        if (main_it != _translators.end() && main_it->first != _current_locale) 
        {
            std::string result = main_it->second->translate(key);
            if (result != key)
                return result;
        }
        
        for (const auto& [name, trans] : _translators) 
        {
            if (name == _current_locale || name == "main") 
                continue;

            std::string result = trans->translate(key);
            if (result != key)
                return result;
        }

        return key;
    }

    template<typename... Args>
    std::string translate(const std::string& key, Args... args) const 
    {
        auto current_it = _translators.find(_current_locale);
        if (current_it != _translators.end()) 
        {
            std::string result = current_it->second->translate(key, args...);
            if (result != key)
                return result;
        }
        
        auto main_it = _translators.find("main");
        if (main_it != _translators.end() && main_it->first != _current_locale) 
        {
            std::string result = main_it->second->translate(key, args...);
            if (result != key)
                return result;
        }
        
        for (const auto& [name, trans] : _translators) 
        {
            if (name == _current_locale || name == "main")
                continue;

            std::string result = trans->translate(key, args...);
            if (result != key)
                return result;
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
        std::string filename = base_name + "_" + _current_locale;
        std::vector<std::string> extensions = {".properties"};
        for (const auto& ext : extensions) 
        {
            std::string file_path = directory_path + "/" + filename + ext;
            if (!std::filesystem::exists(file_path)) 
                continue;

            auto trans = std::make_unique<translator>(_current_locale);
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
    std::string _current_locale;
    std::unordered_map<std::string, std::unique_ptr<translator>> _translators;
};


// ------------------------------- global API ------------------------------------
inline std::string tr(const std::string& key, const std::string& default_text = "") 
{
    return i18n::instance().translate(key, default_text);
}

template<typename... Args>
inline std::string tr(const std::string& key, Args... args) 
{
    return i18n::instance().translate(key, args...);
}

} // namespace hj

#endif // I18N_HPP