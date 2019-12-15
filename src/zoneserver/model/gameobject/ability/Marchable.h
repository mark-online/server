#pragma once

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Marchable
 * 행군할 수 있는가(침략군)
 */
class Marchable
{
public:
    virtual ~Marchable() {}

public:
    virtual void marchStarted() = 0;
    virtual void marchStopped() = 0;

public:
    /// 행군 해야 하는가?
    virtual bool shouldMarch() const = 0;

    /// 행군 중인가?
    virtual bool isMarching() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
