#pragma once


namespace gideon { namespace zoneserver {
class Arena;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Arenaable
 * 투기장을 사용할수 있다
 */
class ArenaMatchable
{
public:
    virtual ~ArenaMatchable() {}
public:
    virtual Arena* getArena() = 0;
		virtual ArenaPoint getArenaPoint() const = 0;
    virtual CharacterArenaPlayResults& getCharacterArenaPlayResults() = 0;

	virtual ErrorCode reviveInArena(const ObjectPosition& position) = 0;

	virtual void setArena(Arena* arena) = 0;
	virtual void upArenaPoint(ArenaPoint arenaPoint) = 0;
	virtual void downArenaPoint(ArenaPoint arenaPoint) = 0;
	virtual void setArenaPoint(ArenaPoint arenaPoint) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
