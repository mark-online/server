#pragma once

#include "../zoneserver_export.h"
#include "Controller.h"
#include "callback/EntityControllerAbility.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/Chatting.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/StatusInfo.h>

namespace gideon { namespace zoneserver {
class WorldMap;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class EntityController
 * Controller for Entity
 */
class ZoneServer_Export EntityController : public Controller,
    public EntityControllerAbility
{
public:
    /// 월드맵에 스폰되었다
    virtual void spawned(WorldMap& worldMap) = 0;

    /// 월드맵에서 디스폰되었다
    virtual void despawned(WorldMap& worldMap) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
