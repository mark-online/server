#pragma once

#include <gideon/Common.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Certificate.h>
#include <gideon/cs/shared/data/ExpelReason.h>
#include <gideon/cs/shared/data/UserInfo.h>

namespace gideon { namespace clientnet {

/**
 * @class LoginSessionCallback
 */
class LoginSessionCallback : public boost::noncopyable
{
public:
	virtual ~LoginSessionCallback() {}

    virtual void expelled(ExpelReason reason) = 0;
};


/**
 * @class CommunitySessionCallback
 */
class CommunitySessionCallback : public boost::noncopyable
{
public:
	virtual ~CommunitySessionCallback() {}

    virtual void communityServerDisconnected() = 0;
};


/**
 * @class ZoneSessionCallback
 */
class ZoneSessionCallback : public boost::noncopyable
{
public:
	virtual ~ZoneSessionCallback() {}

    virtual void zoneServerDisconnected() = 0;
};

}} // namespace gideon { namespace clientnet {
