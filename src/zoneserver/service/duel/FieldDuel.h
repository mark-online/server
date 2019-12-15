#pragma once

#include <gideon/cs/shared/data/DuelInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Coordinate.h>
#include <sne/base/concurrent/Future.h>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}}

namespace gideon { namespace zoneserver {

class DestroyDuelCallback;
/**** 
 * @class FieldDual
 *
 ****/
class FieldDual
{
    typedef std::mutex LockType;
public:
    FieldDual(DestroyDuelCallback* callback, ObjectId duelId,
        go::Entity& duealer1, go::Entity& duealer2);
    ~FieldDual();      

    bool shouldStopByDistance(go::Entity& duealer) const;
    
    void start();
    void stop(FieldDuelResultType type, ObjectId winnerId);

protected:
    void ready();

private:
    mutable LockType lock_;
    DestroyDuelCallback* callback_;
    ObjectId duelId_;
    go::Entity& duealer1_;
    go::Entity& duealer2_;
    Position startPosition_;

    sne::base::Future::WeakRef destroyTask_;
    sne::base::Future::WeakRef startTask_;
    bool isDestory_;
};

typedef std::shared_ptr<FieldDual> FieldDualPtr;

}} // namespace gideon { namespace zoneserver {