#pragma once

#include "../ItemCastable.h"
#include "../Knowable.h"
#include "../../model/item/CastableItem.h"
#include "../../model/gameobject/Entity.h"
#include "../../service/item/CastableItemService.h"
#include <sne/core/container/Containers.h>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class ItemCastableAbility
 * 아이템을 시전할 수 있는 능력
 */
class ItemCastableAbility : public ItemCastable
{
    typedef std::shared_ptr<CastableItem> CastableItemPtr;
    typedef sne::core::HashMap<ObjectId, CastableItemPtr> CastableItemMap;
    
public:
    ItemCastableAbility(Entity& owner) :
        owner_(owner) {}
        
    void finalize() {
        for (CastableItemMap::value_type& value : items_) {
            CastableItem* item = (value.second).get();
            item->cancel();
        }
    }

public:
    virtual void insertCastItem(ObjectId itemId, DataCode dataCode) {
        if (hasItem(itemId)) {
            return;
        }

        std::unique_ptr<CastableItem> item(
            CASTABLE_ITEM_SERVICE->createCastableItemFor(owner_, itemId, dataCode));
        if (!item) {
            return;
        }

        {
            std::lock_guard<Entity::LockType> lock(owner_.getLock());

            items_.emplace(itemId, std::move(item));
        }
    }

    virtual void removeCastItem(ObjectId itemId) {
        std::lock_guard<Entity::LockType> lock(owner_.getLock());

        items_.erase(itemId);
    }

public:
    virtual ErrorCode castTo(const GameObjectInfo& targetInfo, ObjectId objectId) {
        CastableItemPtr item = getCastableItem(objectId);
        if (! item.get()) {
            assert(false);
            return ecItemNotExist;
        }

        return item->castTo(targetInfo);
    }

    virtual ErrorCode castAt(const Position& targetPosition, ObjectId objectId) {
        CastableItemPtr item = getCastableItem(objectId);
        if (! item.get()) {
            assert(false);
            return ecItemNotExist;
        }

        return item->castAt(targetPosition);
    }

    virtual void cancel(ObjectId objectId) {
        CastableItemPtr item = getCastableItem(objectId);
        if (! item.get()) {
            return;
        }

        return item->cancel();
    }

    virtual ErrorCode checkItemCasting(ObjectId itemId,
        const GameObjectInfo& targetInfo) const {
        Knowable* knowable = owner_.queryKnowable();
        if (! knowable) {
            assert(false);
            return ecItemTargetNotFound;
        }

        {
            std::lock_guard<Entity::LockType> lock(owner_.getLock());

            if (! hasItem(itemId)) {
                return ecItemNotExist;
            }

            if (! knowable->doesKnow(targetInfo)) {
                if (! owner_.isSame(targetInfo)) {
                    return ecItemTargetNotFound;
                }
            }
        }
        return ecOk;
    }

    virtual ErrorCode checkItemCasting(ObjectId itemId,
        const Position& targetPosition) const {
        std::lock_guard<Entity::LockType> lock(owner_.getLock());

        if (! hasItem(itemId)) {
            return ecItemNotExist;
        }
        
        // TODO: targetPosition 검사할게 있나?
        targetPosition;
        return ecOk;
    }

    virtual bool checkQuestItem(ObjectId /*objectId*/) const {
        assert(false && "Not Call");
        return false;
    }

    virtual void useQuestItem(ObjectId /*objectId*/) {
        assert(false && "Not Call");
    }

    virtual bool checkElementItem(ObjectId /*objectId*/) const {
        assert(false && "Not Call");
        return false;
    }

    virtual void useElementItem(ObjectId /*objectId*/) {
        assert(false && "Not Call");
    }

private:    
    bool hasItem(ObjectId objectId) const {
        return items_.find(objectId) != items_.end();
    }

    CastableItemPtr getCastableItem(ObjectId objectId) {
        std::lock_guard<Entity::LockType> lock(owner_.getLock());

        const CastableItemMap::iterator pos = items_.find(objectId);
        if (pos == items_.end()) {
            return CastableItemPtr();
        }
        return (*pos).second;
    }

private:
    Entity& owner_;

    CastableItemMap items_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
