#pragma once


namespace gideon { namespace zoneserver {

/**
 * @class ChaoState
 **/
class ChaoState
{
public:
	virtual ~ChaoState() {}

public:
	virtual bool changeChaoState(bool isChaoState) = 0;

    virtual bool changeTempChaoState(bool isChaoState) = 0;

};


}} // namespace gideon { namespace zoneserver {
