#pragma once


namespace gideon { namespace zoneserver {

class MoveState;
class CreatureState;
class SkillCasterState;
class TradeState;
class ItemManageState;
class HarvestState;
class TreasureState;
class GliderState;
class VehicleState;
class ChaoState;
class CastState;
class CombatState;
class FieldDuelState;

/**
 * @class EntityStateAbility
 **/
class EntityStateAbility
{
public:
	virtual MoveState* queryMoveState() {
		return nullptr;
	}
	virtual CreatureState* queryCreatureState() {
		return nullptr;
	}
	virtual SkillCasterState* querySkillCasterState() {
		return nullptr;
	}
	virtual TradeState* queryTradeState() {
		return nullptr;
	}
	virtual ItemManageState* queryItemManageState() {
		return nullptr;
	}
	virtual HarvestState* queryHarvestState() {
		return nullptr;
	}
    virtual TreasureState* queryTreasureState() {
        return nullptr;
    }
    virtual GliderState* queryGliderState() {
        return nullptr;
    }
    virtual VehicleState* queryVehicleState() {
        return nullptr;
    }
    virtual ChaoState* queryChaoState() {
        return nullptr;
    }
    virtual CastState* queryCastState() {
        return nullptr;
    }
    virtual CombatState* queryCombatState() {
        return nullptr;
    }
    virtual FieldDuelState* queryFieldDuelState() {
        return nullptr;
    }
    virtual const FieldDuelState* queryFieldDuelState() const {
        return nullptr;
    }
};


}} // namespace gideon { namespace zoneserver {
