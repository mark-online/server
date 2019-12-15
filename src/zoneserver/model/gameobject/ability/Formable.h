#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>

namespace gideon { namespace zoneserver {

class NpcFormation;

typedef std::shared_ptr<NpcFormation> NpcFormationRefPtr;

}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Formable
 * 하수인을 소환하여 대형을 이룰 수 있다
 */
class Formable
{
public:
    virtual ~Formable() {}

public:
    virtual void formUp() = 0;

    virtual void setFormation(NpcFormationRefPtr formation) = 0;

public:
    virtual bool hasForm() const = 0;

    virtual NpcFormationRefPtr getFormation() = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
