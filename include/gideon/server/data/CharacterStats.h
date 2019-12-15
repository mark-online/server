#pragma once

#include <gideon/cs/shared/data/CharacterInfo.h>

namespace gideon {


/**
 * @struct DBCharacterStats
 * 캐릭터 상태를 디비에 저장
 */
struct DBCharacterStats
{
    CreatureLevel level_;    
    LevelStep levelStep_;
    ExpPoint exp_;
    uint8_t state_; // 전투상태 평화상태만 저장한다
    SkillPoint sp_;
    Points currentPoints_;
    WorldPosition position_;
    ZoneId zoneId_;
	GameMoney gameMoney_;
	GameMoney bankGameMoney_;
    Chaotic chaotic_;
    bool isActionBarLocked_;
    CharacterClass cc_;
    ArenaPoint arenaPoint_;
    EventCoin eventCoin_;
	ForgeCoin forgeCoin_;

    explicit DBCharacterStats(CreatureLevel level = clMinLevel, LevelStep levelStep = lsMaxLevel, 
        ExpPoint exp = ceMin, uint8_t state = cstNormal, SkillPoint sp = spMin, 
        const Points& currentPoints = Points(), 
        const WorldPosition& position = WorldPosition(), ZoneId zoneId = invalidZoneId,
		GameMoney gameMoney = gameMoneyMin, GameMoney bankGameMoney = gameMoneyMin,
        Chaotic chaotic = minChaotic, bool isActionBarLocked = false, 
        CharacterClass cc = ccUnknown, ArenaPoint arenaPoint = 0, 
        EventCoin eventCoin = 0, ForgeCoin forgeCoin = 0) :
        level_(level),
        levelStep_(levelStep),
        exp_(exp),
        state_(state),
        sp_(sp),
        currentPoints_(currentPoints),
        position_(position),
        zoneId_(zoneId),
		gameMoney_(gameMoney),
		bankGameMoney_(bankGameMoney),
        chaotic_(chaotic),
        isActionBarLocked_(isActionBarLocked),
        cc_(cc),
        arenaPoint_(arenaPoint),
        eventCoin_(eventCoin),
		forgeCoin_(forgeCoin)
    {}

    template <typename Stream>
    void serialize(Stream& stream) {
        stream & level_ & levelStep_ & exp_ & state_ & sp_ & currentPoints_ & 
            position_ & zoneId_ & gameMoney_ & bankGameMoney_ & chaotic_ & isActionBarLocked_ &
            cc_ & arenaPoint_ & eventCoin_ & forgeCoin_;
    }
};

} // namespace gideon {
