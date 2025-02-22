#ifndef LEMON_ACCEPTOR_H
#define LEMON_ACCEPTOR_H

#include "lemon/net/channel.h"
#include "lemon/net/socket.h"

namespace lemon {
namespace net {

class EventLoop;
class InetAddress;
class Acceptor {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { m_newConnectionCallback = cb; }

    void listen();
    bool listening() const { return m_listening; }

private:
    void handleRead();

    EventLoop* m_loop;
    Socket m_listen_socket;
    Channel m_channel;
    NewConnectionCallback m_newConnectionCallback;
    bool m_listening;
    int m_idle_fd;
};

} // namespace net
} // namespace lemon

#endif