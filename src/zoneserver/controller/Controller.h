#pragma once

#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace gc {

/**
 * @class Controller
 * Controller
 * - MVC pattern's Controller role
 */
class Controller : public boost::noncopyable
{
public:
    Controller(go::Entity* owner = nullptr) :
        owner_(owner) {}
    virtual ~Controller() {}

    void setOwner(go::Entity& owner) {
        owner_ = &owner;
    }

public:
    go::Entity& getOwner() {
        assert(owner_ != nullptr);
        return *owner_;
    }

    const go::Entity& getOwner() const {
        assert(owner_ != nullptr);
        return *owner_;
    }

    template <typename EntityT>
    EntityT& getOwnerAs() {
        return static_cast<EntityT&>(getOwner());
    }

    template <typename EntityT>
    const EntityT& getOwnerAs() const {
        return static_cast<const EntityT&>(getOwner());
    }

private:
    go::Entity* owner_;
};

}}} // namespace gideon { namespace zoneserver { namespace gc {
