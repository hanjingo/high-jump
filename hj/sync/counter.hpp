#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <limits>
#include <atomic>
#include <mutex>

namespace hj
{

template <typename T>
class counter
{
  public:
    counter(const counter &rhs)
        : _value{rhs._value.load()}
        , _step{rhs._step}
        , _min{rhs._min}
        , _max{rhs._max}
    {
    }
    counter(const T value)
        : _value{value}
        , _step{1}
    {
        _min = std::numeric_limits<T>::min();
        _max = std::numeric_limits<T>::max();
    }
    counter(const T value, const T min)
        : _value{value}
        , _step{1}
        , _min{min}
    {
        _max = std::numeric_limits<T>::max();
    }
    counter(const T value, const T min, const T step)
        : _value{value}
        , _step{step}
        , _min{min}
    {
        _max = std::numeric_limits<T>::max();
    }
    counter(const T value, const T min, const T max, const T step)
        : _value{value}
        , _step{step}
        , _min{min}
        , _max{max}
    {
    }
    ~counter() {}

    inline counter &operator++()
    {
        T tmp;
        do
        {
            tmp = _value.load();
            if(tmp + _step > _max || tmp + _step < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp + _step));

        return *this;
    }

    inline counter operator++(int)
    {
        T tmp;
        do
        {
            tmp = _value.load();
            if(tmp + _step > _max || tmp + _step < _min)
                return counter(tmp, _min, _max, _step);
        } while(!_value.compare_exchange_weak(tmp, tmp + _step));

        return counter(tmp, _min, _max, _step);
    }

    inline counter operator+(const counter &ct)
    {
        T tmp = _value.load();
        T arg = ct._value.load();
        if(tmp + arg > _max || tmp + arg < _min)
            return counter(tmp, _min, _max, _step);

        return counter(tmp + arg, _min, _max, _step);
    }

    inline counter operator+(const T &arg)
    {
        T tmp = _value.load();
        if(tmp + arg > _max || tmp + arg < _min)
            return counter(tmp, _min, _max, _step);

        return counter(tmp + arg, _min, _max, _step);
    }

    inline counter &operator+=(const counter &ct)
    {
        T tmp;
        T arg;
        do
        {
            tmp = _value.load();
            arg = ct._value.load();
            if(tmp + arg > _max || tmp + arg < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp + arg));

        return *this;
    }

    inline counter &operator+=(const T &arg)
    {
        T tmp;
        do
        {
            tmp = _value.load();
            if(tmp + arg > _max || tmp + arg < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp + arg));

        return *this;
    }

    inline counter &operator--()
    {
        T tmp;
        do
        {
            tmp = _value.load();
            if(tmp - _step > _max || tmp - _step < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp - _step));

        return *this;
    }

    inline counter operator--(int)
    {
        T tmp;
        do
        {
            tmp = _value.load();
            if(tmp - _step > _max || tmp - _step < _min)
                return counter(tmp, _min, _max, _step);
        } while(!_value.compare_exchange_weak(tmp, tmp - _step));

        return counter(tmp, _min, _max, _step);
    }

    inline counter operator-(const counter &ct)
    {
        T tmp = _value.load();
        T arg = ct._value.load();
        if(tmp - arg > _max || tmp - arg < _min)
            return counter(tmp, _min, _max, _step);

        return counter(tmp - arg, _min, _max, _step);
    }

    inline counter operator-(const T &arg)
    {
        T tmp = _value.load();
        if(tmp - arg > _max || tmp - arg < _min)
            return counter(tmp, _min, _max, _step);

        return counter(tmp - arg, _min, _max, _step);
    }

    inline counter &operator-=(const counter &ct)
    {
        T tmp;
        T arg;
        do
        {
            tmp = _value.load();
            arg = ct._value.load();
            if(tmp - arg > _max || tmp - arg < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp - arg));

        return *this;
    }

    inline counter &operator-=(const T &arg)
    {
        T tmp;
        do
        {
            tmp = _value.load();
            if(tmp - arg > _max || tmp - arg < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp - arg));

        return *this;
    }

    inline counter operator*(const counter &ct)
    {
        T tmp = _value.load();
        T arg = ct._value.load();
        if(tmp * arg > _max || tmp * arg < _min)
            return counter(tmp, _min, _max, _step);

        return counter(tmp * arg, _min, _max, _step);
    }

    inline counter operator*(const T &arg)
    {
        T tmp = _value.load();
        if(tmp * arg > _max || tmp * arg < _min)
            return counter(tmp, _min, _max, _step);

        return counter(tmp * arg, _min, _max, _step);
    }

    inline counter &operator*=(const counter &ct)
    {
        T tmp;
        T arg;
        do
        {
            tmp = _value.load();
            arg = ct._value.load();
            if(tmp * arg > _max || tmp * arg < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp * arg));

        return *this;
    }

    inline counter &operator*=(const T &arg)
    {
        T tmp;
        do
        {
            tmp = _value.load();
            if(tmp * arg > _max || tmp * arg < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp * arg));

        return *this;
    }

    inline counter &operator=(const counter &ct)
    {
        _min  = ct._min;
        _max  = ct._max;
        _step = ct._step;

        T tmp = _value.load();
        while(!_value.compare_exchange_weak(tmp, ct._value.load()))
        {
            tmp = _value.load();
        }

        return *this;
    }

    inline counter &operator=(const T &value)
    {
        _min  = std::numeric_limits<T>::min();
        _max  = std::numeric_limits<T>::max();
        _step = 1;

        T tmp = _value.load();
        while(!_value.compare_exchange_weak(tmp, value))
        {
            tmp = _value.load();
        }

        return *this;
    }

    inline friend counter operator/(const counter &ct1, const counter &ct2)
    {
        T arg1 = ct1._value.load();
        T arg2 = ct2._value.load();
        if(arg1 / arg2 > ct1._max || arg1 / arg2 < ct1._min)
            return counter(arg1, ct1._min, ct1._max, ct1._step);

        return counter(arg1 / arg2, ct1._min, ct1._max, ct1._step);
    }

    inline friend counter operator/(const counter &ct, const T &arg)
    {
        T value = ct._value.load();
        if(value / arg > ct._max || value / arg < ct._min)
            return counter(value, ct._min, ct._max, ct._step);

        return counter(value / arg, ct._min, ct._max, ct._step);
    }

    inline friend counter operator/(const T &arg, const counter &ct)
    {
        T value = ct._value.load();
        if(arg / value > ct._max || arg / value < ct._min)
            return counter(arg, ct._min, ct._max, ct._step);

        return counter(arg / value, ct._min, ct._max, ct._step);
    }

    inline friend counter &operator/=(counter &ct1, const counter &ct2)
    {
        T arg1;
        T arg2;
        do
        {
            arg1 = ct1._value.load();
            arg2 = ct2._value.load();
            if(arg1 / arg2 > ct1._max || arg1 / arg2 < ct1._min)
                return ct1;
        } while(!ct1._value.compare_exchange_weak(arg1, arg1 / arg2));

        return ct1;
    }

    inline friend counter &operator/=(counter &ct, const T &arg)
    {
        T value;
        do
        {
            value = ct._value.load();
            if(value / arg > ct._max || value / arg < ct._min)
                return ct;
        } while(!ct._value.compare_exchange_weak(value, value / arg));

        return ct;
    }

    inline friend bool operator==(const counter &ct1, const counter &ct2)
    {
        return ct1._value.load() == ct2._value.load();
    }

    inline friend bool operator==(const counter &ct, const T &value)
    {
        return ct._value.load() == value;
    }

    inline friend bool operator!=(const counter &ct1, const counter &ct2)
    {
        return ct1._value.load() != ct2._value.load();
    }

    inline friend bool operator!=(const counter &ct, const T &value)
    {
        return ct._value.load() != value;
    }

    inline friend bool operator<(const counter &ct1, const counter &ct2)
    {
        return ct1._value.load() < ct2._value.load();
    }

    inline friend bool operator<(const counter &ct, const T &value)
    {
        return ct._value.load() < value;
    }

    inline friend bool operator<=(const counter &ct1, const counter &ct2)
    {
        return ct1._value.load() <= ct2._value.load();
    }

    inline friend bool operator<=(const counter &ct, const T &value)
    {
        return ct._value.load() <= value;
    }

    inline friend bool operator>(const counter &ct1, const counter &ct2)
    {
        return ct1._value.load() > ct2._value.load();
    }

    inline friend bool operator>(const counter &ct, const T &value)
    {
        return ct._value.load() > value;
    }

    inline friend bool operator>=(const counter &ct1, const counter &ct2)
    {
        return ct1._value.load() >= ct2._value.load();
    }

    inline friend bool operator>=(const counter &ct, const T &value)
    {
        return ct._value.load() >= value;
    }

    inline friend std::ostream &operator<<(std::ostream &out, const counter &ct)
    {
        out << ct._value.load();
        return out;
    }

  public:
    inline counter &inc()
    {
        T tmp;
        do
        {
            tmp = _value.load();
            if(tmp + _step > _max || tmp + _step < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp + _step));

        return *this;
    }

    inline counter &dec()
    {
        T tmp;
        do
        {
            tmp = _value.load();
            if(tmp - _step > _max || tmp - _step < _min)
                return *this;
        } while(!_value.compare_exchange_weak(tmp, tmp - _step));

        return *this;
    }

    inline const T &step() { return _step; }

    inline const T value() { return _value.load(); }

    inline const T &max() { return _max; }

    inline const T &min() { return _min; }

    inline counter &reset(const T &value = 0)
    {
        if(value > _max || value < _min)
            return *this;

        T tmp;
        do
        {
            tmp = _value.load();
        } while(!_value.compare_exchange_weak(tmp, value));
        return *this;
    }

  private:
    std::atomic<T> _value;
    T              _max;
    T              _min;
    T              _step;
};

}

#endif