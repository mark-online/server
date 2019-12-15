#pragma once


namespace gideon { namespace zoneserver {

/**
 * @class FieldDuelState
 **/
class FieldDuelState
{
public:
	virtual ~FieldDuelState() {}

public:
	virtual bool isFieldDueling() const = 0;
    virtual bool isMyDueler(ObjectId fieldDuelId) const = 0;
    virtual ObjectId getFieldDuelId() const = 0;

    virtual bool startFieldDuel(ObjectId fieldDuelId) = 0;
    virtual bool stopFieldDuel() = 0;
};


}} // namespace gideon { namespace zoneserver {
