#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/GuildInfo.h>
#include <gideon/cs/shared/data/ErrorCode.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class Buildable
 * 설치물을 생성 있다
 */
class Buildable
{
public:
    virtual ~Buildable() {}
	
public:
    virtual ErrorCode build(go::Entity& player) = 0;

public:
    /// 고용된 NPC가 despawn되었다
    virtual void hiringNpcDespawned(ObjectId mercenaryId) = 0;

public:
    /// 건물 소유주(플레이어) ID
    virtual ObjectId getOwnerId() const = 0;

    /// 건물 소유주의 길드 ID
    virtual GuildId getOwnerGuildId() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
