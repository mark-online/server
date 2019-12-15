#include "../../../zoneserver_export.h"
#include "CreatureEffectScriptCommand.h"
#include "PassiveSkillApplierHelper.h"
#include <gideon/server/data/ServerSkillEffect.h>
#include <gideon/cs/shared/data/ErrorCode.h>
#include <gideon/cs/shared/data/SkillEffectInfo.h>
#include <gideon/cs/shared/data/SkillCastableEquipItemTypeInfo.h>
#include <sne/base/concurrent/Future.h>


namespace gideon { namespace datatable {
struct PassiveSkillTemplate;
struct SkillEffectTemplate;
}} //namespace gideon { namespace datatable {

namespace gideon { namespace zoneserver { namespace go {

class Player;
class EffectScriptCommand;
class PassiveSkill;

/**
 * @class PassiveSkillManager
 */
class ZoneServer_Export PassiveSkillManager 
{       
    typedef std::shared_ptr<PassiveSkill> PassiveSkillPtr;
    typedef sne::core::HashMap<ServerEffectType, EffectScriptCommand*> EffectSciptCommandMap;
    typedef sne::core::HashMap<SkillCode, PassiveSkillPtr> PassiveSkillMap;
    typedef sne::core::Vector<PassiveSkillPtr> PassiveSkills;
    typedef sne::core::HashMap<PassiveCheckCondition, PassiveSkills> PassiveSkillsMap;

public:
    typedef sne::core::HashMap<SkillCode, ServerEffectPtr> ServerEffectMap;

public:
    PassiveSkillManager(Creature& owner);
    ~PassiveSkillManager();

    void finalize();

public:
    ErrorCode learn(SkillCode skillCode);
    ErrorCode unlearn(SkillCode skillCode);

    void applySkill(const SkillEffectResult& skillEffectResult);
    void cancelSkill(SkillCode skillCode);
    void revert();

    void tick();

    void notifyChangeCondition(PassiveCheckCondition condition);
    
    const DebuffBuffEffectInfoSet& getDebuffBuffEffectInfoSet() const {
        return debuffBuffEffectInfoSet_;
    }
private:
    void addSkill(const datatable::PassiveSkillTemplate& skillTemplate,
        const datatable::SkillEffectTemplate& effectTemplate);
    void removeSkill(SkillCode skillCode);

    void addConditionSkill(PassiveSkillPtr passiveSkill, PassiveCheckCondition condition);    
    void removeConditionSkill(SkillCode skillCode, PassiveCheckCondition condition);
        
    void removeTimeoutEffects();
    void executeEffects();
    void cancelHasTimeEffect();
    void cancelEffect(ServerEffectPtr effectPtr);
    void excuteEffect(ServerEffectPtr effectPtr);
    void notifyEffectAdded(const DebuffBuffEffectInfo& info);
    void notifyEffectRemoved(DataCode dataCode);


    PassiveSkillPtr getPassiveSkill(SkillCode skillCode);
    PassiveSkills& getPassiveSkills(PassiveCheckCondition condition);
    SkillCode getSkillCode(SkillIndex index) const;

private:
    EffectScriptCommand* getEffectInfoCommand(EffectScriptType type);

private:
    PassiveSkillMap passiveSkillMap_;
    PassiveSkillsMap conditionSkillMap_;
    EffectSciptCommandMap commandMap_;
    ServerEffectMap timeEffectMap_;
    ServerEffectMap onceApplyEffectMap_;
    DebuffBuffEffectInfoSet debuffBuffEffectInfoSet_;
    Creature& owner_;
};


}}} // namespace gideon { namespace zoneserver { namespace go {
