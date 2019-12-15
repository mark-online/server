#pragma once

namespace gideon { namespace zoneserver {
class WorldEventPlayCallback;
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
* @class WorldEventPlayable
 * 
 */
class WorldEventPlayable
{
public:
    WorldEventPlayable() {}
    virtual ~WorldEventPlayable() {}
	
public:
    virtual void setWorldEventPlayCallback(WorldEventPlayCallback* callback) = 0;    
    virtual WorldEventPlayCallback* getWorldEventPlayCallback() = 0;
   
};

}}} // namespace gideon { namespace zoneserver { namespace go {
