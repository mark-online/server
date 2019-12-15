#pragma once

#include <gideon/cs/shared/data/Time.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace go {
class Npc;
}}} // namespace gideon { namespace zoneserver { namespace go {

namespace gideon { namespace zoneserver { namespace ai {

class Brain;

/**
 * @class GameScriptConstructor
 */
template <typename ST>
class GameScriptConstructor
{
public:
    GameScriptConstructor() {
        new ST();
    }
};


/**
 * @class GameScript
 */
class GameScript
{
public:
    GameScript(const char* name) :
        name_(name) {}
    virtual ~GameScript() {}

public:
    const std::string& getName() const {
        return name_;
    }

private:
    std::string name_;
};


/**
 * @class UpdatableScript
 */
class UpdatableScript
{
public:
    virtual ~UpdatableScript() {}

    virtual void update(GameTime diff) = 0;
};


/**
 * @class NpcScript
 */
class NpcScript : public GameScript
{
protected:
    NpcScript(const char* name);

public:
    virtual std::unique_ptr<Brain> createBrain(go::Npc& owner,
        uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4) = 0;
};


/**
 * @class GameScriptManager
 */
class GameScriptManager : public boost::noncopyable
{
public:
    static GameScriptManager& instance();

public:
    virtual ~GameScriptManager() {}

    std::unique_ptr<Brain> createBrain(go::Npc& owner);
};

}}} // namespace gideon { namespace zoneserver { namespace ai {

#define GAME_SCRIPT_MANAGER gideon::zoneserver::ai::GameScriptManager::instance()
