#pragma once

#include "../Factionable.h"
#include "../../Npc.h"
#include <gideon/cs/datatable/FactionTable.h>
#include <gideon/cs/datatable/NpcTable.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class FactionableAbility
 * 파벌 인식 능력
 * - Faction이 동적으로 변할 수 있으므로, FactionTemplate을 캐싱하지 않음
 */
class FactionableAbility : public Factionable
{
    typedef std::mutex LockType;

public:
    FactionableAbility(Entity& owner) :
        owner_(owner),
        factionCode_(invalidFactionCode) {}

private:
    virtual void setFaction(FactionCode factionCode) {
        assert(isValidFactionCode(factionCode));
        factionCode_ = factionCode;
    }

    virtual bool isFriendlyTo(const Entity& target) const {
        assert(isValidFactionCode(factionCode_));
        const datatable::FactionTemplate* targetFactionTemplate =
            getFactionTemplate(target);
        if (! targetFactionTemplate) {
            return false;
        }

        const datatable::FactionTemplate* myFactionTemplate =
            getFactionTemplate(factionCode_);
        if (! myFactionTemplate) {
            assert(false);
            return false;
        }

        return myFactionTemplate->isFriendlyTo(*targetFactionTemplate);
    }

    virtual bool isHostileTo(const Entity& target) const {
        assert(isValidFactionCode(factionCode_));
        const datatable::FactionTemplate* targetFactionTemplate =
            getFactionTemplate(target);
        if (! targetFactionTemplate) {
            return false;
        }

        const datatable::FactionTemplate* myFactionTemplate =
            getFactionTemplate(factionCode_);
        if (! myFactionTemplate) {
            assert(false);
            return false;
        }

        return myFactionTemplate->isHostileTo(*targetFactionTemplate);
    }

    virtual FactionCode getFactionCode() const {
        return factionCode_;
    }

private:
    const datatable::FactionTemplate* getFactionTemplate(const Entity& target) const {
        const Factionable* targetFactionable =
            const_cast<Entity&>(target).queryFactionable();
        if (! targetFactionable) {
            return nullptr;
        }
        return getFactionTemplate(targetFactionable->getFactionCode());
    }

    const datatable::FactionTemplate* getFactionTemplate(FactionCode factionCode) const {
        const datatable::FactionTemplate* factionTemplate =
            FACTION_TABLE->getFaction(factionCode);
        assert(factionTemplate != nullptr);
        return factionTemplate;
    }

private:
    Entity& owner_;
    FactionCode factionCode_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
