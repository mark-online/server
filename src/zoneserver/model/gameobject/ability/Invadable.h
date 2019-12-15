#pragma once

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Invadable
 * 침략할 수 있는가?
 */
class Invadable
{
public:
    virtual ~Invadable() {}

public:
    /// 침략군인가?
    virtual bool isInvader() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {