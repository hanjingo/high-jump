#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <limits>
#include <atomic>
#include <mutex>

namespace libcpp
{

template<typename T>
class counter
{
public:
    counter(const counter& rhs) 
        : value_{rhs.value_.load()}, 
          step_{rhs.step_},
          min_{rhs.min_},
          max_{rhs.max_}
    {}
    counter(const T value) 
        : value_{value}, 
          step_{1}
    {
        min_ = std::numeric_limits<T>::min();
        max_ = std::numeric_limits<T>::max();
    }
    counter(const T value, const T min) 
        : value_{value}, 
          step_{1},
          min_{min}
    {
        max_ = std::numeric_limits<T>::max();
    }
    counter(const T value, const T min, const T step) 
        : value_{value}, 
          step_{step},
          min_{min}
    {
        max_ = std::numeric_limits<T>::max();
    }
    counter(const T value, const T min, const T max, const T step) 
        : value_{value}, 
          step_{step},
          min_{min},
          max_{max}
    {}
    ~counter() {}

    inline counter& operator++()
    {
        T tmp;
        do {
            tmp = value_.load();
            if (tmp + step_ > max_ || tmp + step_ < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp + step_));

        return *this;
    }

    inline counter operator++(int)
    {
        T tmp;
        do {
            tmp = value_.load();
            if (tmp + step_ > max_ || tmp + step_ < min_)
                return counter(tmp, min_, max_, step_);
        } while(!value_.compare_exchange_weak(tmp, tmp + step_));

        return counter(tmp, min_, max_, step_);
    }

    inline counter operator+(const counter& ct)
    {
        T tmp = value_.load();
        T arg = ct.value_.load();
        if (tmp + arg > max_ || tmp + arg < min_)
            return counter(tmp, min_, max_, step_);
            
        return counter(tmp + arg, min_, max_, step_);
    }

    inline counter operator+(const T& arg)
    {
        T tmp = value_.load();
        if (tmp + arg > max_ || tmp + arg < min_)
            return counter(tmp, min_, max_, step_);
            
        return counter(tmp + arg, min_, max_, step_);
    }

    inline counter& operator+=(const counter& ct)
    {
        T tmp; T arg;
        do {
            tmp = value_.load();
            arg = ct.value_.load();
            if (tmp + arg > max_ || tmp + arg < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp + arg));

        return *this;
    }

    inline counter& operator+=(const T& arg)
    {
        T tmp;
        do {
            tmp = value_.load();
            if (tmp + arg > max_ || tmp + arg < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp + arg));

        return *this;
    }

    inline counter& operator--()
    {
        T tmp;
        do {
            tmp = value_.load();
            if (tmp - step_ > max_ || tmp - step_ < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp - step_));

        return *this;
    }

    inline counter operator--(int)
    {
        T tmp;
        do {
            tmp = value_.load();
            if (tmp - step_ > max_ || tmp - step_ < min_)
                return counter(tmp, min_, max_, step_);
        } while(!value_.compare_exchange_weak(tmp, tmp - step_));

        return counter(tmp, min_, max_, step_);
    }

    inline counter operator-(const counter& ct)
    {
        T tmp = value_.load();
        T arg = ct.value_.load();
        if (tmp - arg > max_ || tmp - arg < min_)
            return counter(tmp, min_, max_, step_);
            
        return counter(tmp - arg, min_, max_, step_);
    }

    inline counter operator-(const T& arg)
    {
        T tmp = value_.load();
        if (tmp - arg > max_ || tmp - arg < min_)
            return counter(tmp, min_, max_, step_);
            
        return counter(tmp - arg, min_, max_, step_);
    }

    inline counter& operator-=(const counter& ct)
    {
        T tmp; T arg;
        do {
            tmp = value_.load();
            arg = ct.value_.load();
            if (tmp - arg > max_ || tmp - arg < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp - arg));

        return *this;
    }

    inline counter& operator-=(const T& arg)
    {
        T tmp;
        do {
            tmp = value_.load();
            if (tmp - arg > max_ || tmp - arg < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp - arg));

        return *this;
    }

    inline counter operator*(const counter& ct)
    {
        T tmp = value_.load();
        T arg = ct.value_.load();
        if (tmp * arg > max_ || tmp * arg < min_)
            return counter(tmp, min_, max_, step_);

        return counter(tmp * arg, min_, max_, step_);
    }

    inline counter operator*(const T& arg)
    {
        T tmp = value_.load();
        if (tmp * arg > max_ || tmp * arg < min_)
            return counter(tmp, min_, max_, step_);

        return counter(tmp * arg, min_, max_, step_);
    }

    inline counter& operator*=(const counter& ct)
    {
        T tmp; T arg;
        do {
            tmp = value_.load();
            arg = ct.value_.load();
            if (tmp * arg > max_ || tmp * arg < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp * arg));

        return *this;
    }

    inline counter& operator*=(const T& arg)
    {
        T tmp;
        do {
            tmp = value_.load();
            if (tmp * arg > max_ || tmp * arg < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp * arg));

        return *this;
    }

    inline counter& operator=(const counter& ct)
    {
        min_ = ct.min_;
        max_ = ct.max_;
        step_ = ct.step_;

        T tmp = value_.load();
        while (!value_.compare_exchange_weak(tmp, ct.value_.load()))
        {
            tmp = value_.load();
        }

        return *this;
    }

    inline counter& operator=(const T& value)
    {
        min_ = std::numeric_limits<T>::min();
        max_ = std::numeric_limits<T>::max();
        step_ = 1;

        T tmp = value_.load();
        while (!value_.compare_exchange_weak(tmp, value))
        {
            tmp = value_.load();
        }

        return *this;
    }

    inline friend counter operator/(const counter& ct1, const counter& ct2)
    {
        T arg1 = ct1.value_.load();
        T arg2 = ct2.value_.load();
        if (arg1 / arg2 > ct1.max_ || arg1 / arg2 < ct1.min_)
            return counter(arg1, ct1.min_, ct1.max_, ct1.step_);

        return counter(arg1 / arg2, ct1.min_, ct1.max_, ct1.step_);
    }

    inline friend counter operator/(const counter& ct, const T& arg)
    {
        T value = ct.value_.load();
        if (value / arg > ct.max_ || value / arg < ct.min_)
            return counter(value, ct.min_, ct.max_, ct.step_);

        return counter(value / arg, ct.min_, ct.max_, ct.step_);
    }

    inline friend counter operator/(const T& arg, const counter& ct)
    {
        T value = ct.value_.load();
        if (arg / value > ct.max_ || arg / value < ct.min_)
            return counter(arg, ct.min_, ct.max_, ct.step_);

        return counter(arg / value, ct.min_, ct.max_, ct.step_);
    }

    inline friend counter& operator/=(counter& ct1, const counter& ct2)
    {
        T arg1; T arg2;
        do {
            arg1 = ct1.value_.load();
            arg2 = ct2.value_.load();
            if (arg1 / arg2 > ct1.max_ || arg1 / arg2 < ct1.min_)
                return ct1;
        } while(!ct1.value_.compare_exchange_weak(arg1, arg1 / arg2));

        return ct1;
    }

    inline friend counter& operator/=(counter& ct, const T& arg)
    {
        T value;
        do {
            value = ct.value_.load();
            if (value / arg > ct.max_ || value / arg < ct.min_)
                return ct;
        } while(!ct.value_.compare_exchange_weak(value, value / arg));

        return ct;
    }

    inline friend bool operator==(const counter& ct1, const counter& ct2)
    {
        return ct1.value_.load() == ct2.value_.load();
    }

    inline friend bool operator==(const counter& ct, const T& value)
    {
        return ct.value_.load() == value;
    }

    inline friend bool operator!=(const counter& ct1, const counter& ct2)
    {
        return ct1.value_.load() != ct2.value_.load();
    }

    inline friend bool operator!=(const counter& ct, const T& value)
    {
        return ct.value_.load() != value;
    }

    inline friend bool operator<(const counter& ct1, const counter& ct2)
    {
        return ct1.value_.load() < ct2.value_.load();
    }

    inline friend bool operator<(const counter& ct, const T& value)
    {
        return ct.value_.load() < value;
    }

    inline friend bool operator<=(const counter& ct1, const counter& ct2)
    {
        return ct1.value_.load() <= ct2.value_.load();
    }

    inline friend bool operator<=(const counter& ct, const T& value)
    {
        return ct.value_.load() <= value;
    }

    inline friend bool operator>(const counter& ct1, const counter& ct2)
    {
        return ct1.value_.load() > ct2.value_.load();
    }

    inline friend bool operator>(const counter& ct, const T& value)
    {
        return ct.value_.load() > value;
    }

    inline friend bool operator>=(const counter& ct1, const counter& ct2)
    {
        return ct1.value_.load() >= ct2.value_.load();
    }

    inline friend bool operator>=(const counter& ct, const T& value)
    {
        return ct.value_.load() >= value;
    }

    inline friend std::ostream& operator<<(std::ostream& out, const counter& ct)
    {
        out << ct.value_.load();
        return out;
    }

public:
    inline counter& inc()
    {
        T tmp;
        do {
            tmp = value_.load();
            if (tmp + step_ > max_ || tmp + step_ < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp + step_));

        return *this;
    }

    inline counter& dec()
    {
        T tmp;
        do {
            tmp = value_.load();
            if (tmp - step_ > max_ || tmp - step_ < min_)
                return *this;
        } while(!value_.compare_exchange_weak(tmp, tmp - step_));

        return *this;
    }

    inline const T& step()
    {
        return step_;
    }

    inline const T value()
    {
        return value_.load();
    }

    inline const T& max()
    {
        return max_;
    }

    inline const T& min()
    {
        return min_;
    }

    inline counter& reset(const T& value = 0)
    {
        if (value > max_ || value < min_)
            return *this;

        T tmp;
        do {
            tmp = value_.load();
        } while(!value_.compare_exchange_weak(tmp, value));
        return *this;
    }

private:
    std::atomic<T> value_;
    T max_;
    T min_;
    T step_;
};

}

#endif