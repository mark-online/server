#pragma once

#include "../../zoneserver_export.h"
#include "FieldDuel.h"
#include "DestoryDuelCallback.h"
#include <gideon/cs/shared/data/ErrorCode.h>
#include <sne/core/utility/Singleton.h>


namespace gideon { namespace zoneserver { namespace go {
class Entity;
}}}

namespace gideon { namespace zoneserver {


/**
 * @class DuelService
 *
 * 결투 서비스
 */
class ZoneServer_Export DuelService : public boost::noncopyable,
    public DestroyDuelCallback
{
    SNE_DECLARE_SINGLETON(DuelService);

    typedef std::mutex LockType;
    typedef sne::core::Map<ObjectId, FieldDualPtr> FieldDualPtrMap;
public:
    DuelService();
    ~DuelService();

    bool initialize();

    ErrorCode canFieldDueling(go::Entity& src, go::Entity& desc) const;
    void startFieldDueling(go::Entity& duealer1, go::Entity& duealer2);

private:
    void setFieldDuelInfo(FieldDualPtr ptr, go::Entity& duealer);

private:
    virtual void destory(ObjectId objectId);
private:
    mutable LockType lock_;
    ObjectId duelId_;
    FieldDualPtrMap ptrMap_;
};


}} // namespace gideon { namespace zoneserver {


#define DUEL_SERVICE gideon::zoneserver::DuelService::instance()
