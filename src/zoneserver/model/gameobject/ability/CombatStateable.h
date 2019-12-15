#pragma once

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class CombatStateable
 * 전투 자세를 가지고 있다.
 */
class CombatStateable
{
public:
    virtual ~CombatStateable() {}
	
public:
	virtual bool changeCombatState(bool isCombatState) = 0;

	virtual bool updateAttackTime(GameTime cooltime = 0) = 0;

	virtual void updateLockPeaceTime(GameTime delayTime) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
