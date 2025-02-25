#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <iostream>
#include <system_error>

namespace libcpp
{

template<typename Value>
class error
{
public:
    error() = delete;
    error(const error<Value>& rhs) : value_{rhs.value_} {}
    error(Value value) : value_{value} {}

    ~error() {}

    inline error& operator=(const error& e)
    {
        value_ = e.value_;
        return *this;
    }

    inline error& operator=(const Value& arg)
    {
        value_ = arg;
        return *this;
    }

    inline friend bool operator==(const error& e1, const error& e2)
    {
        return e1.value_ == e2.value_;
    }

    inline friend bool operator!=(const error& e1, const error& e2)
    {
        return e1.value_ != e2.value_;
    }

    inline friend std::ostream& operator<<(std::ostream& out, const error& e)
    {
        out << e.value_;
        return out;
    }

    inline Value value()
    {
        return value_;
    }

protected:
    Value value_;
};

template<typename Value, typename Handler = void*>
class error_v2 : public error<Value>
{
public:
    error_v2() = delete;
    error_v2(const error_v2<Value, Handler>& rhs) : error<Value>(rhs), handler_{(void*)(rhs.handler_)} {}
    error_v2(Value value) : error<Value>(value), handler_{nullptr} {}
    error_v2(Value value, Handler handler) : error<Value>(value), handler_{(void*)(handler)} {}
    ~error_v2() {}

    template<typename Ret = void, typename... Types>
    Ret handle(Types&& ... args)
    {
        try {
            auto fn = (Ret(*)(Types...))(handler_);
            return fn(std::forward<Types>(args)...);
        } catch (std::exception e) {
            // DO NOTHING
        }
    }
private:
    void* handler_ = nullptr;
};

class error_factory
{
public:
    error_factory() {}
    ~error_factory() {}

    template <typename Value>
    static error<Value> create(Value value)
    {
        return error<Value>(value);
    }

    template <typename Value, typename Handler = void*>
    static error_v2<Value, Handler> create(Value value, Handler handler)
    {
        return error_v2<Value, Handler>(value, (void*)(handler));
    }
};

#define ERROR(...) \
    libcpp::error_factory::create(__VA_ARGS__)

}

#endif