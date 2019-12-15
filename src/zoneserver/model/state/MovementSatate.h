#pragma once


namespace gideon { namespace zoneserver {

/**
 * @class MovementState
 **/
class MovementState
{
public:
	virtual ~MovementState() {}

public:
	virtual bool canMove() const = 0;
	
public:
	virtual void moved() const = 0;
};


}} // namespace gideon { namespace zoneserver {
