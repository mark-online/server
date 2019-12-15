#pragma once


namespace gideon { namespace zoneserver { namespace gc {
class CreatureEffectController;
}}} // namespace gideon { namespace zoneserver { namespace gc {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Effectable
 * 이펙트 효과를 입을수 있다.
 */
class Effectable
{
public:
    virtual ~Effectable() {}
	
public:
    virtual std::unique_ptr<gc::CreatureEffectController> createEffectController() = 0;

public:
	virtual gc::CreatureEffectController& getEffectController() = 0;
	virtual const gc::CreatureEffectController& getEffectController() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
