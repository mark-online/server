#pragma once

#include "../../../zoneserver_export.h"
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/shared/data/ItemEffectInfo.h>
#include <gideon/server/data/ServerSkillEffect.h>
#include <sne/base/concurrent/Future.h>

namespace gideon {
struct RemainEffectInfo;
} // namespace gideon {

namespace gideon { namespace zoneserver { namespace go {

class Entity;
class EffectScriptCommand;

/**
 * @class EffectScriptApplier
 * active skill
 */
class ZoneServer_Export EffectScriptApplier : boost::noncopyable
{
public:
    typedef sne::core::HashMap<ServerEffectType, EffectScriptCommand*> EffectSciptCommandMap;

    EffectScriptApplier(Entity& owner);
    virtual ~EffectScriptApplier();

    void initialize();

    void destroy();
    void clean();
    void saveDB();
	void revert();

    void applySkill(Entity& from, const SkillEffectResult& skillEffectResult);    
    void applyRemainEffect(const RemainEffectInfo& remainEffect);
    void applyEffectScript(EffectScriptType scriptType, GameTime activateTime);

    bool hasEffectScipts(EffectScriptType effectScriptType) const;
    bool hasEffectStackCategory(EffectStackCategory category) const;

    bool isApplyEffect(EffectStackCategory category, EffectLevel skillLevel);
    bool isEmptyCommand() const {
		return commandMap_.empty();
	}

	void cancelRemoveEffect(EffectStackCategory category);
    
public:
    virtual void tick();

protected:
    void addEffect(ServerEffectPtr effectPtr, bool isNotify = true);
    void cancelEffect(ServerEffectPtr effectPtr);

    void removeEffect(EffectStackCategory category);
    void removeTimeoutEffects();
    void executeEffects();

    void addHasTimeEffect(ServerEffectPtr effectInfo);

    virtual void excuteEffect(ServerEffectPtr skillEffect) = 0;
    virtual void notifyEffectAdded(const DebuffBuffEffectInfo& info, bool isNotify) = 0;
    virtual void notifyEffectRemoved(DataCode dataCode, bool isCaster) = 0;

protected:
    virtual bool canExecuteEffect() const = 0;
    virtual void initialzeEffectCommand() = 0; 

protected:
    bool hasEffectStackCategory_i(EffectStackCategory category) const {
        return effectMap_.find(category) != effectMap_.end();
    }

    EffectScriptCommand* getEffectInfoCommand(EffectScriptType type);
    ServerEffectPtr getServerEffectPtr(EffectStackCategory category);

protected:
    go::Entity& getOwner() {
        return owner_;
    }

    const go::Entity& getOwner() const {
        return owner_;
    }

protected:
    template <typename EntityT>
    EntityT& getOwnerAs() {
        return static_cast<EntityT&>(getOwner());
    }

    template <typename EntityT>
    const EntityT& getOwnerAs() const {
        return static_cast<const EntityT&>(getOwner());
    }

protected:
    Entity& owner_;
    EffectSciptCommandMap commandMap_;
    ServerEffectMap effectMap_;
    ServerEffects effects_;
    uint32_t effectId_;
};


}}} // namespace gideon { namespace zoneserver { namespace go {
