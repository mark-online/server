#pragma once


namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Protectionable
 * 보호받을수 있다
 */
class Protectionable
{
public:
    virtual ~Protectionable() {}

public:
	virtual bool isBeginnerProtection() const = 0;

	virtual void releaseBeginnerProtection(bool shouldNotify = true) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {