#pragma once

#include "../../../service/duel/FieldDuel.h"

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class FieldDuelable
 * 필드 1:1대전을 할수있다
 */
class FieldDuelable
{
public:
    virtual ~FieldDuelable() {}
	
public:
    virtual void setFieldDuel(FieldDualPtr ptr = FieldDualPtr()) = 0;

    virtual FieldDualPtr getFieldDualPtr() = 0;

};

}}} // namespace gideon { namespace zoneserver { namespace go {
