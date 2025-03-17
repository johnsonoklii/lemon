#include "lemon/net/tcpconnection.h"
#include "lemon/net/socket.h"
#include "lemon/net/eventloop.h"
#include "lemon/net/buffer.h"

#include "lemon/base/logger/logger.h"

#include <sys/epoll.h>

using namespace lemon;
using namespace lemon::net;
using namespace lemon::log;

TcpConnection::TcpConnection(EventLoop* loop,
    const std::string& name,
    int sockfd,
    int triMode,
    const InetAddress& localAddr,
    const InetAddress& peerAddr)
: m_loop(loop)
 , m_name(name)
 , m_state(kConnecting)
 , m_reading(true)
 , m_triMode(triMode)
 , m_socket(new Socket(sockfd))
 , m_channel(new Channel(loop, sockfd))
 , m_localAddr(localAddr)
 , m_peerAddr(peerAddr)
 , m_highWaterMark(64 * 1024 * 1024)
 , m_lastReadTime(Timestamp::now())
 , m_timeout(-1) {

    if (m_triMode == 0) {
        m_channel->setReadCallback(std::bind(&TcpConnection::handleReadLT, this, std::placeholders::_1));
        m_channel->setWriteCallback(std::bind(&TcpConnection::handleWriteLT, this));
    } else {
        m_socket->setNonBlock();
        m_channel->setTriMode(EPOLLET);
        m_channel->setReadCallback(std::bind(&TcpConnection::handleReadET, this, std::placeholders::_1));
        m_channel->setWriteCallback(std::bind(&TcpConnection::handleWriteET, this));
    }
    
    m_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    m_channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_DEBUG("TcpConnection::ctor[%s] at fd=%d\n", m_name.c_str(), m_channel->fd());
    m_socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG("TcpConnection::dtor[%s] at fd=%d\n", m_name.c_str(), m_channel->fd());
    assert(m_state == kDisconnected);
}

// usually be called in main loop thread
void TcpConnection::startRead() {
    m_loop->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop() {
    m_loop->assertInLoopThread();
    if (!m_reading || !m_channel->isReading()) {
        m_channel->enableReading();
        m_reading = true;
    }
}

// usually be called in main loop thread
void TcpConnection::stopRead() {
    m_loop->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop() {
    m_loop->assertInLoopThread();
    if (m_reading || m_channel->isReading()) {
        m_channel->disableReading();
        m_reading = false;
    }
}

void TcpConnection::connectEstablished() {
    m_loop->assertInLoopThread();
    assert(m_state == kConnecting);
    setState(kConnected);
    if (m_connectionCallback) {
        m_connectionCallback(shared_from_this());
    }
    m_channel->tie(shared_from_this());
    m_channel->enableReading();
}

void TcpConnection::connectDestroyed() {
    m_loop->assertInLoopThread();
    if (m_state == kConnected) {
        setState(kDisconnected);
        m_channel->disableAll();
        if (m_connectionCallback) {
            m_connectionCallback(shared_from_this());
        }
    }
    m_channel->remove();
} 

void TcpConnection::send(const std::string& message) {
    if (m_state == kConnected) {
        m_loop->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
    }
}

void TcpConnection::sendInLoop(const std::string& message) {
    m_loop->assertInLoopThread();
    bool faultError = false;
    ssize_t nwrote = 0;
    size_t remainning = message.size();
    const char *data = message.c_str();

    if (m_state == kDisconnected) {
        LOG_WARN("disconnected, give up writing\n");
        return;
    }

    // 第一次写: 未注册写事件 && 没有待写数据
    if (!m_channel->isWriting() && m_outputBuffer.readableBytes() == 0) {
        nwrote = ::write(m_channel->fd(), data, remainning);
        if (nwrote >= 0) {
            remainning -= nwrote;
            if (remainning == 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            } 
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop\n");
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    assert(remainning <= message.size());

    // 还有未写的数据，放到m_outputBuffer，注册写事件
    if (!faultError && remainning > 0) {
        size_t oldLen = m_outputBuffer.readableBytes();
        if (oldLen < m_highWaterMark 
            && oldLen + remainning >= m_highWaterMark 
            && m_highWaterMarkCallback) {
            m_loop->runInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), oldLen + remainning));        
        }
    
        m_outputBuffer.append(data + nwrote, remainning);
        if (!m_channel->isWriting()) {
            m_channel->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if (m_state == kConnected) {
        setState(kDisconnecting);
        m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    m_loop->assertInLoopThread();
    if (!m_channel->isWriting()) {
        m_socket->shutdownWrite();
    }
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    m_loop->assertInLoopThread();
    int saveErrno = 0;
    ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &saveErrno);
    if (n > 0) {
        m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead\n");
        handleError();
    }   
}

void TcpConnection::handleReadLT(Timestamp receiveTime) {
    m_loop->assertInLoopThread();
    int saveErrno = 0;
    ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &saveErrno);
    if (n > 0) {
        /*
            FIXME: 必须是shared_from_this
            如果conn超时3s没发送数据，主线程会删除conn(析构)，
            但在删除前,conn又发送了一个消息，导致进入了read方法，
            此时如果主线程析构了conn，就会出错
        */ 
        m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
        // FIXME: 加锁？
        m_lastReadTime = receiveTime;
    } else if (n == 0) {
        handleClose();
    } else {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead\n");
        handleError();
    }   
}

void TcpConnection::handleReadET(Timestamp receiveTime) {
    m_loop->assertInLoopThread();
    int saveErrno = 0;
    while (true) {
        ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &saveErrno);
        if (n > 0) {
            m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
            // FIXME: 加锁？
            m_lastReadTime = receiveTime;
        } else if (n == 0) {
            handleClose();
            break;
        } else {
            errno = saveErrno;
            if (errno == EAGAIN || errno == EWOULDBLOCK) { // 数据已读完
                
            } else if (errno == ECONNRESET) { // 连接重置
                LOG_ERROR("TcpConnection fd=%d read error: Connection reset by peer\n", m_channel->fd());
                handleClose(); 
            } else { // 其他错误
                LOG_ERROR("TcpConnection fd=%d read error: %s\n", m_channel->fd(), strerror(errno));
                handleError();
            }
            break;
        }
    }
}

void TcpConnection::handleWrite() {
    m_loop->assertInLoopThread();

    if (m_channel->isWriting()) {
        ssize_t n = ::write(m_channel->fd(), m_outputBuffer.peek(), m_outputBuffer.readableBytes());
        if (n > 0) {
            m_outputBuffer.retrieve(n);
            if (m_outputBuffer.readableBytes() == 0) {
                m_channel->disableWriting();
                if (m_writeCompleteCallback) {
                    m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                }
                if (m_state == kDisconnecting) {
                    shutdownInLoop();
                }   
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite\n");
        }
    } else {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing\n", m_channel->fd());
    }
}

void TcpConnection::handleWriteLT() {
    m_loop->assertInLoopThread();

    if (m_channel->isWriting()) {
        ssize_t n = ::write(m_channel->fd(), m_outputBuffer.peek(), m_outputBuffer.readableBytes());
        if (n > 0) {
            m_outputBuffer.retrieve(n);
            if (m_outputBuffer.readableBytes() == 0) {
                m_channel->disableWriting();
                if (m_writeCompleteCallback) {
                    m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                }
                if (m_state == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else if (n == 0) {
            LOG_WARN("TcpConnection fd=%d write returned 0\n", m_channel->fd());
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // TODO: 在 LT 模式下，不需要重新注册写事件
            } else if (errno == EPIPE || errno == ECONNRESET) {
                LOG_ERROR("TcpConnection fd=%d write error: %s\n", m_channel->fd(), strerror(errno));
                handleClose();
            } else {
                LOG_ERROR("TcpConnection fd=%d write error: %s\n", m_channel->fd(), strerror(errno));
            }
        }
    } else {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing\n", m_channel->fd());
    }
}

void TcpConnection::handleWriteET() {
    m_loop->assertInLoopThread();

    while (true) {
        if (m_channel->isWriting()) {
            ssize_t n = ::write(m_channel->fd(), m_outputBuffer.peek(), m_outputBuffer.readableBytes());
            if (n > 0) {
                m_outputBuffer.retrieve(n);
                if (m_outputBuffer.readableBytes() == 0) {
                    m_channel->disableWriting();
                    if (m_writeCompleteCallback) {
                        m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                    }
                    if (m_state == kDisconnecting) {
                        shutdownInLoop();
                    }   
                    break;
                }
            } else if (n == 0) { 
                LOG_WARN("TcpConnection fd=%d write returned 0\n", m_channel->fd());
                if (m_outputBuffer.readableBytes() > 0) {
                    m_channel->enableWriting();
                }
                break;
            } else {
                // TODO: 在ET模式下，如果不可写，需要重新注册可写事件
                if ((errno == EAGAIN || errno == EWOULDBLOCK) && m_outputBuffer.readableBytes() > 0) {
                    m_channel->enableWriting();
                } else if (errno == EPIPE || errno == ECONNRESET) {
                    LOG_ERROR("TcpConnection fd=%d write error: %s\n", m_channel->fd(), strerror(errno));
                    handleClose();
                } else {
                    LOG_ERROR("TcpConnection fd=%d write error: %s\n", m_channel->fd(), strerror(errno));
                }
                break;
            }
        } else {
            LOG_ERROR("TcpConnection fd=%d is down, no more writing\n", m_channel->fd());
            break;
        }
    }
}

void TcpConnection::handleClose() {
    m_loop->assertInLoopThread();
    assert(m_state == kConnected || m_state == kDisconnecting);

    setState(kDisconnected);
    m_channel->disableAll(); // FIXME: LT模式下，这里必须清除所有事件，否则对端关闭时，会一直触发EPOLLIN事件
    
    TcpConnectionPtr guardThis(shared_from_this());
    if (m_connectionCallback) {
        m_connectionCallback(guardThis);
    }

    if (m_closeCallback) {
        m_closeCallback(guardThis);
    }
}

void TcpConnection::handleError() {
    int err = 0;

    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(m_channel->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    } else {
        err = optval;
    }

    LOG_ERROR("TcpConnection::handleError name=%s - SO_ERROR=%d\n", m_name.c_str(), err);
}


void TcpConnection::forceClose() {
    if (m_state == kConnected || m_state == kDisconnecting) {
        m_loop->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == kConnected || m_state == kDisconnecting) {
        handleClose();
    }
}