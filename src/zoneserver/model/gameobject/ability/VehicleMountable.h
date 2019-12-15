#pragma once

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class VehicleMountable
 * 탈 수 있다
 */
class VehicleMountable
{
public:
    virtual ~VehicleMountable() {}

public:
    virtual void setRiding(bool isRiding) = 0;

    virtual bool isRiding() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
