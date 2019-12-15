#include "ZoneServerPCH.h"
#include "Brain.h"
#include "aggro/AggroList.h"
#include "../model/gameobject/Entity.h"
#include "../model/gameobject/ability/Knowable.h"
#include "../model/gameobject/ability/Thinkable.h"
#include "../model/gameobject/ability/Formable.h"
#include "../service/formation/NpcFormation.h"

namespace gideon { namespace zoneserver { namespace ai {

Brain::Brain(go::Entity& owner) :
    owner_(owner)
{
}


void Brain::notifyToSummons(const go::Entity& attacker, int eventId)
{
    go::Summonable* summonable = owner_.querySummonable();
    if (! summonable) {
        return;
    }

    AggroList* summonerAggroList = getAggroList();
    go::Knowable* knowable = owner_.queryKnowable(); assert(knowable != nullptr);
    for (const GameObjectInfo& summonInfo : summonable->getSummons()) {
        go::Entity* summon = knowable->getEntity(summonInfo);
        if (! summon) {
            summonable->removeSummon(summonInfo);
            continue;
        }
        go::Thinkable* thinkable = summon->queryThinkable();
        if (! thinkable) {
            continue;
        }
        AggroList* aggroList = thinkable->getBrain().getAggroList();
        if (! aggroList) {
            continue;
        }

        // FYI: 랜덤하게 목표를 설정한다
        const go::Entity* target = nullptr;
        if (summonerAggroList != nullptr) {
            target = summonerAggroList->selectRandomVictim();
        }

        if (! target) {
            target = &attacker;
        }

        if (eventId >= 0) {
            aggroList->addThreatByEvent(*target, eventId);
        }
        else {
            aggroList->addThreat(*target, 10, 0);
        }
    }
}


void Brain::notifyToSummoner(const go::Entity& attacker, int eventId)
{
    go::Summonable* summonable = owner_.querySummonable();
    if (! summonable) {
        return;
    }

    const GameObjectInfo summonerInfo = summonable->getSummoner();
    if (! summonerInfo.isValid()) {
        return;
    }

    go::Knowable* knowable = owner_.queryKnowable(); assert(knowable != nullptr);
    go::Entity* summoner = knowable->getEntity(summonerInfo);
    if (! summoner) {
        return;
    }

    go::Thinkable* thinkable = summoner->queryThinkable();
    if (! thinkable) {
        return;
    }
    AggroList* aggroList = thinkable->getBrain().getAggroList();
    if (! aggroList) {
        return;
    }
    aggroList->addThreatByEvent(attacker, eventId);

    thinkable->getBrain().notifyToSummons(attacker, eventId);
}

}}} // namespace gideon { namespace zoneserver { namespace ai {
