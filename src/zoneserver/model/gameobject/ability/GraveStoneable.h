#pragma once


namespace gideon { namespace zoneserver { namespace gc {
class PlayerGraveStoneController;
}}} // namespace gideon { namespace zoneserver { namespace gc {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class GraveStoneable
 * 바석을 사용할수 있다
 */
class GraveStoneable
{
public:
    virtual ~GraveStoneable() {}
	
public:
	virtual std::unique_ptr<gc::PlayerGraveStoneController> createPlayerGraveStoneController() = 0;

public:
	virtual gc::PlayerGraveStoneController& getPlayerGraveStoneController() = 0;
	virtual const gc::PlayerGraveStoneController& getPlayerGraveStoneController() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
