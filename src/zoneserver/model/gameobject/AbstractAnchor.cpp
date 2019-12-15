#include "ZoneServerPCH.h"
#include "AbstractAnchor.h"
#include "../time/CoolDownTimer.h"
#include "../time/GlobalCoolDownTimer.h"
#include "skilleffect/StaticObjectEffectScriptApplier.h"
#include "status/StaticObjectStatus.h"
#include "ability/Guildable.h"
#include "ability/impl/KnowableAbility.h"
#include "ability/impl/SkillCastableAbility.h"
#include "ability/impl/CastNotificationableImpl.h"
#include "../../world/WorldMap.h"
#include "../../controller/EntityController.h"
#include "../../service/time/GameTimer.h"
#include "../../service/skill/helper/SOEffectHelper.h"
#include "../../service/guild/GuildService.h"
#include <gideon/cs/datatable/AnchorTable.h>
#include <gideon/3d/3d.h>
#include <sne/base/concurrent/TaskScheduler.h>
#include <sne/core/memory/MemoryPoolMixin.h>

namespace gideon { namespace zoneserver { namespace go {

namespace
{

/**
 * @class AutoSkillCastTask
 */
class AutoSkillCastTask : public sne::base::Runnable,
    public sne::core::ThreadSafeMemoryPoolMixin<AutoSkillCastTask>
{
public:
    AutoSkillCastTask(SkillCastable& castable,
        const GameObjectInfo& anchorInfo, StaticObjectSkillCode skillCode) :
        castable_(castable),
        anchorInfo_(anchorInfo),
        skillCode_(skillCode) {}

private:
    virtual void run() {
        if (! anchorInfo_.isValid()) {
            return;
        }
        if (castable_.canCast(skillCode_)) {
            castable_.castTo(anchorInfo_, skillCode_);
        }
    }

private:
    SkillCastable& castable_;
    const GameObjectInfo& anchorInfo_;
    StaticObjectSkillCode skillCode_;    
};

}

AbstractAnchor::AbstractAnchor(std::unique_ptr<gc::EntityController> controller) :
    Parent(std::move(controller)),
    globalCoolTime_(std::make_unique<GlobalCoolDownTimer>())
{
    coolTime_= std::make_unique<CoolDownTimer>(*this);
    knowable_= std::make_unique<KnowableAbility>(*this);
	castNotificationable_= std::make_unique<CastNotificationableImpl>(*this);
    skillCastableAbility_= std::make_unique<SkillCastableAbility>(*this);
	oneSecondTracker_.reset(1000);
}


AbstractAnchor::~AbstractAnchor()
{
}


void AbstractAnchor::finalize()
{
    std::lock_guard<LockType> lock(getLock());

	if (skillCastableAbility_.get()) {
		skillCastableAbility_->cancelAll();
	}

    if (knowable_.get()) {
        knowable_->forgetAll();
    }

    Parent::finalize();
}



ErrorCode AbstractAnchor::despawn()
{
    const ErrorCode errorCode = Parent::despawn();
    if (isFailed(errorCode)) {
        assert(false);
        return errorCode;
    }

    finalize();
    return ecOk;
}


std::unique_ptr<EffectScriptApplier> AbstractAnchor::createEffectScriptApplier()
{
    return std::make_unique<StaticObjectEffectScriptApplier>(*this);
}


std::unique_ptr<EffectHelper> AbstractAnchor::createEffectHelper()
{
    return std::make_unique<SoEffectHelper>(*this);
}


void AbstractAnchor::tick(GameTime diff)
{
	getEffectScriptApplier().tick();

	oneSecondTracker_.update(diff);
	if (! oneSecondTracker_.isPassed()) {
		return;
	}
	oneSecondTracker_.reset(1000);

	handleExpiredTasks();
}



void AbstractAnchor::fillOwnerInfo(BuildingOwnerInfo& ownerInfo, BuildingOwnerType ownerType, go::Entity& player)
{
    ownerInfo.ownerType_ = ownerType;
    if (isGuildOwnerType(ownerType)) {
        GuildId guildId = player.queryGuildable()->getGuildId();
        if (! isValidGuildId(guildId)) {
            return;
        }
        GuildPtr guild = GUILD_SERVICE->getGuildByGuildId(guildId);
        if (guild.get() != nullptr) {
            ownerInfo.guildInfo_= guild->getBaseGuildInfo();
        }

    }
    else if (isPrivateOwnerType(ownerType)) {
        ownerInfo.playerOwnerInfo_.objectId_ = player.getObjectId();
        ownerInfo.playerOwnerInfo_.nickname_ = player.getNickname();
    }

}


void AbstractAnchor::setCooldown(DataCode dataCode, GameTime coolTime,
    uint32_t index, GameTime globalCoolDownTime)
{
    globalCoolTime_->setNextGlobalCooldownTime(index, globalCoolDownTime);
    coolTime_->startCooldown(dataCode, coolTime);
}


void AbstractAnchor::cancelCooldown(DataCode dataCode, uint32_t index)
{
    globalCoolTime_->cancelCooldown(index);
    coolTime_->cancelCooldown(dataCode);    
}


void AbstractAnchor::cancelPreCooldown()
{
	globalCoolTime_->cancelPreCooldown();
	coolTime_->cancelPreCooldown();    
}


bool AbstractAnchor::isGlobalCooldown(uint32_t index) const
{
    const GameTime currentTime = GAME_TIMER->msec();

    const msec_t nextGlobalCooldownTime = 
        globalCoolTime_->getNextGlobalCooldownTime(index);
    if (nextGlobalCooldownTime == 0) {
        return false;
    }
    return currentTime < nextGlobalCooldownTime;
}


bool AbstractAnchor::isLocalCooldown(DataCode dataCode) const
{
    return coolTime_->isCooldown(dataCode);;
}

// = Positionable overriding

void AbstractAnchor::setPosition(const ObjectPosition& position)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setPosition(position);
   
    getAnchorPosition() = position;
}


void AbstractAnchor::setHeading(Heading heading)
{
    std::lock_guard<LockType> lock(getLockPositionable());

    Parent::setHeading(heading);

    getAnchorPosition().heading_ = heading;
}


WorldPosition AbstractAnchor::getWorldPosition() const
{
    std::lock_guard<LockType> lock(getLockPositionable());

    const WorldMap* worldMap = getCurrentWorldMap();
    if (! worldMap) {
        return WorldPosition();
    }

    return WorldPosition(getAnchorPosition(), worldMap->getMapCode());
}


ErrorCode AbstractAnchor::castTo(const GameObjectInfo& targetInfo, SkillCode skillCode)
{
    return skillCastableAbility_->castTo(targetInfo, skillCode);
}


ErrorCode AbstractAnchor::castAt(const Position& targetPosition, SkillCode skillCode)
{    
    return skillCastableAbility_->castAt(targetPosition, skillCode);
}


void AbstractAnchor::cancel(SkillCode skillCode)
{
    skillCastableAbility_->cancel(skillCode);
}


void AbstractAnchor::cancelConcentrationSkill(SkillCode /*skillCode*/)
{
}


void AbstractAnchor::cancelAll()
{
    skillCastableAbility_->cancelAll();
}


void AbstractAnchor::consumePoints(const Points& points)
{
    if (points.hp_ > 0) {
        getStaticObjectStatus().reduceHp(points.hp_);
    }
}


void AbstractAnchor::consumeMaterialItem(const BaseItemInfo& /*itemInfo*/)
{

}


ErrorCode AbstractAnchor::checkSkillCasting(SkillCode skillCode,
    const GameObjectInfo& targetInfo) const
{
    return skillCastableAbility_->checkSkillCasting(skillCode, targetInfo);
}


ErrorCode AbstractAnchor::checkSkillCasting(SkillCode skillCode,
    const Position& targetPosition) const
{
    return skillCastableAbility_->checkSkillCasting(skillCode, targetPosition);
}



float32_t AbstractAnchor::getLongestSkillDistance() const
{
    return skillCastableAbility_->getLongestSkillDistance();
}


bool AbstractAnchor::isUsing(SkillCode skillCode) const
{
    return skillCastableAbility_->isUsing(skillCode);
}


bool AbstractAnchor::canCast(SkillCode skillCode) const
{
    if (! isActiveAbillity()) {
        return true;
    }
	// 쿨타임 체크
    return skillCastableAbility_->canCast(skillCode);
}

}}} // namespace gideon { namespace zoneserver { namespace go {
