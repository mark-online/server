#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/CooldownInfo.h>
#include <gideon/cs/shared/data/SkillInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/core/container/Containers.h>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} //namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver {


/***
 * @class CoolDownTimer
 ***/
class CoolDownTimer
{
    typedef sne::core::Map<DataCode, GameTime> CoolDownInfoMap;
    typedef sne::core::Map<SkillIndex, GameTime> SkillCoolDownInfoMap;
public:
    CoolDownTimer(go::Entity& entity);
    ~CoolDownTimer();
    void finalize();
	void reset();
    void saveDB();
    void setCooldown(const CooltimeInfos& cooltimeInfos);
    bool isCooldown(DataCode dataCode) const;
	void cancelPreCooldown();
    void startCooldown(DataCode dataCode, GameTime downTime);
    void cancelCooldown(DataCode dataCode);

private:
    GameTime getCoolDownTime(DataCode dataCode) const;

private:
    go::Entity& owner_;
    CoolDownInfoMap timeMap_;
    SkillCoolDownInfoMap skillCoolDownTimeMap_;
	DataCode preCooddownCode_;
};

}} // namespace gideon { namespace zoneserver {