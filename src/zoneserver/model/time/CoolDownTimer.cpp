#include "ZoneServerPCH.h"
#include "CoolDownTimer.h"
#include "../../service/time/GameTimer.h"
#include "../gameobject/Entity.h"
#include "../../controller/EntityController.h"
#include "../../controller/callback/CooldownCallback.h"

namespace gideon { namespace zoneserver {

namespace {

bool isSkillCode(DataCode dataCode)
{
    return ctSkill == getCodeType(dataCode);
}

} // namespace {


CoolDownTimer::CoolDownTimer(go::Entity& entity) :
    owner_(entity),
	preCooddownCode_(invalidDataCode)
{
}


CoolDownTimer::~CoolDownTimer()
{
    finalize();
}


void CoolDownTimer::finalize()
{
    timeMap_.clear();
    skillCoolDownTimeMap_.clear();
}


void CoolDownTimer::reset()
{
	timeMap_.clear();
	skillCoolDownTimeMap_.clear();
    preCooddownCode_ = invalidDataCode;
}


void CoolDownTimer::saveDB()
{
    if (! owner_.isValid()) {
        return;
    }

    gc::CooldownCallback* callback = owner_.getController().queryCooldownCallback();
    if (! callback) {
        return;
    }

    GameTime gameTimeNow = GAME_TIMER->msec();
    GameTime saveTime = GAME_TIMER->msec() + (cooldownSaveTime * 1000);
    sec_t localNow = getTime();
    CooltimeInfos timeInfos;
    for (const CoolDownInfoMap::value_type& value : timeMap_) {
        GameTime expireTime = value.second;
        if (expireTime > saveTime) {
            sec_t leftTime = (expireTime - gameTimeNow) / 1000;
            timeInfos.push_back(CooltimeInfo(value.first, localNow + leftTime));
        }
    }

    if (! timeInfos.empty()) {
        callback->cooldownInfosAdded(timeInfos);
    }    
}


void CoolDownTimer::setCooldown(const CooltimeInfos& cooltimeInfos)
{
    sec_t localNowTime = getTime();
    for (const CooltimeInfo& info : cooltimeInfos) {
        if (info.endTime_ > localNowTime) {
            GameTime leftCoolTime = GameTime((info.endTime_ - localNowTime) * 1000);
            startCooldown(info.dataCode_, leftCoolTime);
        }        
    }
}


bool CoolDownTimer::isCooldown(DataCode dataCode) const
{
    GameTime coolDownTime = getCoolDownTime(dataCode);   
    if (coolDownTime == 0) {
        return false;
    }

    GameTime now = GAME_TIMER->msec();
    return now < coolDownTime;
}


void CoolDownTimer::cancelPreCooldown()
{
	cancelCooldown(preCooddownCode_);
}


void CoolDownTimer::startCooldown(DataCode dataCode, GameTime downTime)
{
    if (dataCode == invalidDataCode || downTime == 0) {
        return;
    }    

    GameTime coolDownTime = GAME_TIMER->msec() + downTime;

    if (isSkillCode(dataCode)) {
        SkillIndex index = getSkillIndex(dataCode);
        SkillCoolDownInfoMap::iterator pos = skillCoolDownTimeMap_.find(index);
        if (pos != skillCoolDownTimeMap_.end()) {
            (*pos).second = coolDownTime;
        }
        else {
            skillCoolDownTimeMap_.emplace(index, coolDownTime);
        }
    }
    
    CoolDownInfoMap::iterator pos = timeMap_.find(dataCode);
    if (pos != timeMap_.end()) {
        (*pos).second = coolDownTime;
    }
    else {
        timeMap_.emplace(dataCode, coolDownTime);
    }
	preCooddownCode_ = dataCode;
}


void CoolDownTimer::cancelCooldown(DataCode dataCode)
{
    if (dataCode == invalidDataCode) {
        return;
    }

    if (isSkillCode(dataCode)) {
        SkillIndex index = getSkillIndex(dataCode);
        SkillCoolDownInfoMap::iterator pos = skillCoolDownTimeMap_.find(index);
        if (pos != skillCoolDownTimeMap_.end()) {
            (*pos).second = 0;
        }
    }
    
    CoolDownInfoMap::iterator pos = timeMap_.find(dataCode);
    if (pos != timeMap_.end()) {
        (*pos).second = 0;
    }
}


GameTime CoolDownTimer::getCoolDownTime(DataCode dataCode) const
{
    if (isSkillCode(dataCode)) {
        SkillIndex index = getSkillIndex(dataCode);
        SkillCoolDownInfoMap::const_iterator pos = skillCoolDownTimeMap_.find(index);
        if (pos != skillCoolDownTimeMap_.end()) {
            return (*pos).second;
        }
    }
    else {
        CoolDownInfoMap::const_iterator pos = timeMap_.find(dataCode);
        if (pos != timeMap_.end()) {

            return (*pos).second;
        }
    }

    return 0;
}

}} // namespace gideon { namespace zoneserver {