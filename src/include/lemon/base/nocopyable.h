#ifndef LEMON_NOCOPYABLE_H
#define LEMON_NOCOPYABLE_H

namespace lemon {
namespace base {
    
class noncopyable {
public:
    noncopyable() = default;
    ~noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

} // namespace base
} // namespace lemon

#endif