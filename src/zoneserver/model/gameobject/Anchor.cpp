#include "ZoneServerPCH.h"
#include "Anchor.h"
#include "Entity.h"
#include "ability/Inventoryable.h"
#include "ability/impl/KnowableAbility.h"
#include "ability/impl/SkillCastableAbility.h"
#include "ability/impl/CastNotificationableImpl.h"
#include "status/StaticObjectStatus.h"
#include "skilleffect/StaticObjectEffectScriptApplier.h"
#include "../time/GlobalCooldownTimer.h"
#include "../time/CooldownTimer.h"
#include "../item/LootItemInventory.h"
#include "../bank/BankAccount.h"
#include "../../world/World.h"
#include "../../world/WorldMap.h"
#include "../../controller/EntityController.h"
#include "../../service/skill/helper/SOEffectHelper.h"
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/3d/3d.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace go {

Anchor::Anchor(std::unique_ptr<gc::EntityController> controller) :
    StaticObject(std::move(controller)),
    expireTime_(0)
{
    coolTime_= std::make_unique<CoolDownTimer>(*this);
    knowable_= std::make_unique<KnowableAbility>(*this);
    castNotificationable_= std::make_unique<CastNotificationableImpl>(*this);
    skillCastableAbility_= std::make_unique<SkillCastableAbility>(*this);
    oneSecondTracker_.reset(1000);
}


Anchor::~Anchor()
{
}


bool Anchor::initialize(DataCode dataCode, ObjectId objectId,
    const ObjectPosition& position, go::Entity* player)
{
    std::lock_guard<LockType> lock(getLock());

    if (! StaticObject::initialize(otAnchor, objectId)) {
        return false;
    }

    anchorTemlate_ = ANCHOR_TABLE->getAnchorTemplate(dataCode);
    if (!anchorTemlate_) {
        return false;
    }

    anchorInfo_.objectId_ = objectId;
    anchorInfo_.objectType_ = otAnchor;
    anchorInfo_.anchorCode_ = dataCode;
    anchorInfo_.position_ = position;

    initStaticObjectStatusInfo(anchorTemlate_->getStaticObjectStatusInfo());

    if (player) {
        anchorInfo_.ownerInfo_.ownerId_ = player->getObjectId();
        anchorInfo_.ownerInfo_.nickname_ = player->getNickname();
    }

    skillCastableAbility_->learn(anchorTemlate_->getSkillCode());

    UnionEntityInfo& entityInfo = getUnionEntityInfo_i();
    entityInfo.objectType_ = getObjectType();
    MoreAnchorInfo& moreAnchorInfo = entityInfo.asAnchorInfo();
    static_cast<BaseAnchorInfo&>(moreAnchorInfo) = anchorInfo_;
    const GameTime now = GAME_TIMER->msec();
    if (0 < anchorTemlate_->getLiveSec()) {
        expireTime_ = now + static_cast<GameTime>(anchorTemlate_->getLiveSec() * 1000);
    }

    if (0 < anchorTemlate_->getActiveInterval()) {
        nextActivateTime_ = now + anchorTemlate_->getActiveInterval();
    }
    return true;
}


void Anchor::finalize()
{
    {
        std::lock_guard<LockType> lock(getLock());

        anchorInfo_.reset();
    }

    StaticObject::finalize();
}


std::unique_ptr<EffectScriptApplier> Anchor::createEffectScriptApplier()
{
    return std::make_unique<StaticObjectEffectScriptApplier>(*this);
}


std::unique_ptr<EffectHelper> Anchor::createEffectHelper()
{
    return std::make_unique<SoEffectHelper>(*this);
}


void Anchor::tick(GameTime diff)
{
    getEffectScriptApplier().tick();

    oneSecondTracker_.update(diff);
    if (! oneSecondTracker_.isPassed()) {
        return;
    }
    oneSecondTracker_.reset(1000);

    handleExpiredTasks();
}


void Anchor::handleExpiredTasks()
{
    const GameTime now = GAME_TIMER->msec();
    
    if (getStaticObjectStatus().isMinHp() || expireTime_ < now) {
        despawn();
        return;
    }
        
    if (nextActivateTime_ < now) {
        nextActivateTime_ += anchorTemlate_->getActiveInterval();
        castTo(getGameObjectInfo(), anchorTemlate_->getSkillCode());
    }    
}


void Anchor::setCooldown(DataCode dataCode, GameTime coolTime,
    uint32_t index, GameTime globalCoolDownTime)
{
    assert(globalCoolTime_.get() != nullptr && "TODO: 왜 초기화하지 않았는가?");
    globalCoolTime_->setNextGlobalCooldownTime(index, globalCoolDownTime);
    coolTime_->startCooldown(dataCode, coolTime);
}


void Anchor::cancelCooldown(DataCode dataCode, uint32_t index)
{
    globalCoolTime_->cancelCooldown(index);
    coolTime_->cancelCooldown(dataCode);    
}


void Anchor::cancelPreCooldown()
{
    globalCoolTime_->cancelPreCooldown();
    coolTime_->cancelPreCooldown();    
}


bool Anchor::isGlobalCooldown(uint32_t index) const
{
    const GameTime currentTime = GAME_TIMER->msec();

    const msec_t nextGlobalCooldownTime = 
        globalCoolTime_->getNextGlobalCooldownTime(index);
    if (nextGlobalCooldownTime == 0) {
        return false;
    }
    return currentTime < nextGlobalCooldownTime;
}


bool Anchor::isLocalCooldown(DataCode dataCode) const
{
    return coolTime_->isCooldown(dataCode);;
}



// = Positionable overriding

WorldPosition Anchor::getWorldPosition() const
{
    std::lock_guard<LockType> lock(getLockPositionable());

    const WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return WorldPosition();
    }

    return WorldPosition(getPosition(), worldMap->getMapCode());
}

// = SkillCastable overriding
ErrorCode Anchor::castTo(const GameObjectInfo& targetInfo, SkillCode skillCode)
{
    return skillCastableAbility_->castTo(targetInfo, skillCode);
}


ErrorCode Anchor::castAt(const Position& targetPosition, SkillCode skillCode)
{    
    return skillCastableAbility_->castAt(targetPosition, skillCode);
}


void Anchor::cancel(SkillCode skillCode)
{
    skillCastableAbility_->cancel(skillCode);
}


void Anchor::cancelConcentrationSkill(SkillCode /*skillCode*/)
{
}


void Anchor::cancelAll()
{
    skillCastableAbility_->cancelAll();
}


void Anchor::consumePoints(const Points& points)
{
    if (points.hp_ > 0) {
        getStaticObjectStatus().reduceHp(points.hp_);
    }
}


void Anchor::consumeMaterialItem(const BaseItemInfo& /*itemInfo*/)
{

}


ErrorCode Anchor::checkSkillCasting(SkillCode skillCode,
    const GameObjectInfo& targetInfo) const
{
    return skillCastableAbility_->checkSkillCasting(skillCode, targetInfo);
}


ErrorCode Anchor::checkSkillCasting(SkillCode skillCode,
    const Position& targetPosition) const
{
    return skillCastableAbility_->checkSkillCasting(skillCode, targetPosition);
}



float32_t Anchor::getLongestSkillDistance() const
{
    return skillCastableAbility_->getLongestSkillDistance();
}


bool Anchor::isUsing(SkillCode skillCode) const
{
    return skillCastableAbility_->isUsing(skillCode);
}


bool Anchor::canCast(SkillCode skillCode) const
{
    // TODO: Anchor
    //if (! isActiveAbillity()) {
    //    return true;
    //}
    // 쿨타임 체크
    return skillCastableAbility_->canCast(skillCode);
}

}}} // namespace gideon { namespace zoneserver { namespace go {
