#pragma once

namespace gideon { namespace zoneserver { namespace go {


/**
 * @class WeaponUseable
 * 무기를 사용 있다
 */
class WeaponUseable
{
public:
    virtual ~WeaponUseable() {}
	
public:
    virtual bool isMeleeWeaponEquipped() const = 0;
    virtual bool isShieldEquipped() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
