#include "ZoneServerPCH.h"
#include "GameTimer.h"
#include <sne/core/utility/SystemUtil.h>

namespace gideon { namespace zoneserver {

namespace {

/**
 * @class DefaultGameTimerSource
 */
class DefaultGameTimerSource : public GameTimerSource
{
private:
    virtual GameTime msec() const override {
        return sne::core::getTickCount();
    }
};

} // namespace

SNE_DEFINE_SINGLETON(GameTimer);

GameTimer::GameTimer(std::unique_ptr<GameTimerSource> source) :
    source_(std::move(source))
{
    if (! source_) {
        source_ = std::make_unique<DefaultGameTimerSource>();
    }
}

}} // namespace gideon { namespace zoneserver {
