#pragma once

namespace gideon { 
struct BaseItemInfo;
} //namespace gideon { 

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class Buildable
 * 설치물을 생성 있다
 */
class ResourcesProductionable
{
public:
    virtual ~ResourcesProductionable() {}
	
public:
    virtual void productionResources(const BaseItemInfo& itemInfo) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
