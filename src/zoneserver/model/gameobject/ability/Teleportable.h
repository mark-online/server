#pragma once


namespace gideon { namespace zoneserver { namespace go {

/**
* @class Teleportable
 * .
 */
class Teleportable
{
public:
    virtual ~Teleportable() {}
	
public:
	virtual const Nickname& getCallee() const = 0;
	virtual const WorldPosition& getRecallPosition() const = 0;
	virtual const BindRecallInfo* getBindRecallInfo(ObjectId linkId) const = 0;

	virtual void setRecallPosition(const WorldPosition& worldPosition) = 0;
	virtual void setCallee(const Nickname& nickname) = 0;
	virtual void removeBindRecall(ObjectId linkId) = 0;
	
	virtual ErrorCode addBindRecall(const BindRecallInfo& info) = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
