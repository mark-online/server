#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/datatable/SpawnTemplate.h>

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class Summonable
 * 소환하거나 소환 당할 수 있다
 */
class Summonable
{
public:
    typedef sne::core::Set<GameObjectInfo> EntitySet;

public:
    virtual ~Summonable() {}
	
public:
    virtual void setSummoner(go::Entity& summoner, SpawnType spawnType) = 0;
    virtual void unsetSummoner() = 0;

    virtual void addSummon(const GameObjectInfo& summon) = 0;
    virtual void removeSummon(const GameObjectInfo& summon) = 0;

    virtual EntitySet getSummons() const = 0;
    virtual GameObjectInfo getSummoner() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
