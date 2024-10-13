#ifndef _NOCOPYABLE_H_
#define _NOCOPYABLE_H_

namespace deric
{
class Nocopyable
{
protected:
    Nocopyable() = default;

    ~Nocopyable() = default;

    Nocopyable(const Nocopyable&) = delete;

    Nocopyable& operator=(const Nocopyable&) = delete;
};
}

#endif