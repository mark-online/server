#pragma once

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class Bankable
 * 은행기능이 있다
 */
class Bankable
{
public:
    virtual ~Bankable() {}
	
public:
    virtual ErrorCode canBankable(go::Entity& player) const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
