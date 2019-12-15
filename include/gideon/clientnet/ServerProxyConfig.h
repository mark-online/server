#ifndef CLIENTNET_SERVERPROXYCONFIG_H
#define CLIENTNET_SERVERPROXYCONFIG_H

#pragma once

#include <gideon/Common.h>

namespace gideon { namespace clientnet {

/**
 * @struct ServerProxyConfig
 * ServerProxy 클래스를 생성하기 위한 설정
 */
struct ServerProxyConfig
{
    enum {
        defaultConnectionTimeout = 10 * 1000, ///< 10초
        defaultHeartbeatInterval = 3 * 1000, ///< 3초
    };

    /// 로그인 서버 IP 주소
    std::string loginServerAddress_;

    /// 로그인 서버 포트 번호
    uint16_t loginServerPort_;

    /// 서버 연결 제한 시간 (밀리 초 단위)
    uint16_t connectionTimeout_;

    /// 패킷 암호화 알고리즘("aes", "tea", "none")
    std::string packetCipherAlorightm_;

    /// 허트비트 패킷 전송 간격 (밀리초 단위)
    uint16_t heartbeatInterval_;

    /// polling 방식?? (ServerProxy::handleMessages()를 호출하여야 한다!)
    bool usePolling_;

    ServerProxyConfig() :
        connectionTimeout_(defaultConnectionTimeout),
        packetCipherAlorightm_("aes"),
        heartbeatInterval_(defaultHeartbeatInterval),
        usePolling_(false) {}

    bool isValid() const {
        return (! loginServerAddress_.empty()) && (loginServerPort_ > 0);
    }
};

}} // namespace gideon { namespace clientnet {

#endif // CLIENTNET_SERVERPROXYCONFIG_H
