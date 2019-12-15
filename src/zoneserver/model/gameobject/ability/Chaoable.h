#pragma once


namespace gideon { namespace zoneserver { namespace go {

/**
* @class Chaoable
 * 범죄자를 만들수 있다.
 */
class Chaoable
{
public:
    virtual ~Chaoable() {}
	
public:
	virtual Chaotic getChaotic() const = 0;

	virtual bool isChao() const = 0;

    virtual bool isTempChao() const = 0;

    virtual bool isRealChao() const = 0;

    virtual void hitToPlayer() = 0;

    virtual void standGraveStone() = 0;

	virtual void downChaotic(Chaotic chaotic) = 0;

	virtual void upChaotic(Chaotic chaotic) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
