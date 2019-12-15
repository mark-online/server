#pragma once


namespace gideon { namespace zoneserver { namespace go {
class LifeStatusCallback;
}}} // namespace gideon { namespace zoneserver { namespace go {


namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class LifeStatusCallback 
 */
class LifeStatusCallback
{
public:
    virtual ~LifeStatusCallback() {}

    virtual void died(go::Entity* entity) = 0;
};


}}} //namespace gideon { namespace zoneserver { namespace gc {
