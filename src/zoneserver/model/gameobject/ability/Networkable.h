#pragma once


namespace gideon { namespace zoneserver { namespace go {

/**
* @class Networkable
 * .
 */
class Networkable
{
public:
    virtual ~Networkable() {}
	
public:
	virtual void logout() = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
