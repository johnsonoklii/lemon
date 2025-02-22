#ifndef LEMON_CALLBACKS_H
#define LEMON_CALLBACKS_H

#include "lemon/base/timestamp.h"

#include <functional>
#include <memory>

namespace lemon{
namespace net {

using namespace lemon::base;

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;

using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;
} // namespace net
} // namespace lemon
#endif