#pragma once

namespace gideon { namespace zoneserver {

namespace go {
class Entity;
} //namespace go {
}} // namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace ai {

class Brain;

/**
 * @struct BrainFactory
 */
struct BrainFactory
{
    static std::unique_ptr<Brain> createBrain(go::Entity& owner);
};

}}} // namespace gideon { namespace zoneserver { namespace ai {
