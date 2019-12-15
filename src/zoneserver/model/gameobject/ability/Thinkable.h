#pragma once

namespace gideon { namespace zoneserver { namespace ai {
class Brain;
}}} // namespace gideon { namespace zoneserver { namespace ai {

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class Thinkable
 * 생각할 수 있는가?
 * - AI
 */
class Thinkable
{
public:
    virtual bool hasWalkRoutes() const = 0;

    virtual ai::Brain& getBrain() = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
