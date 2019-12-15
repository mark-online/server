#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>
#include <boost/noncopyable.hpp>

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class GameObject
 * 월드 상의 모든 객체의 부모 클래스
 */
class GameObject : public boost::noncopyable
{
public:
    GameObject() {}
    virtual ~GameObject() {}

protected:
    virtual bool initialize(ObjectType objectType, ObjectId objectId) {
        assert(! isValid());
        info_.objectType_ = objectType;
        info_.objectId_ = objectId;
        return isValid();
    }

    virtual void finalize() {
        info_.reset();
    }

public:
    bool isValid() const {
        return info_.isValid();
    }

    const GameObjectInfo& getGameObjectInfo() const {
        return info_;
    }

    ObjectType getObjectType() const {
        return info_.objectType_;
    }

    ObjectId getObjectId() const {
        return info_.objectId_;
    }

public:
    bool isCreature() const {
        return info_.isCreature();
    }

    bool isStaticObject() const {
        return info_.isStaticObject();
    }

    bool isPlayer() const {
        return info_.isPlayer();
    }

	bool isGraveStone() const {
		return info_.isGraveStone();
	}

    bool isNpc() const {
        return info_.isNpc();
    }

    bool isNpcOrMonster() const {
        return info_.isNpcOrMonster();
    }

    bool isMonster() const {
        return info_.isMonster();
    }

    bool isDungeon() const {
        return info_.objectType_ == otDungeon;
    }

	bool isHarvest() const {
		return info_.objectType_ == otHarvest;
	}

    bool isTreasure() const {
        return info_.objectType_ == otTreasure;
    }

    bool isAnchor() const {
        return info_.objectType_ == otAnchor;
    }

    bool isBuilding() const {
        return info_.objectType_ == otBuilding;
    }

	bool isDevice() const {
		return info_.objectType_ == otDevice;
	}

    bool isAnchorOrBuilding() const {
        return isAnchor() || isBuilding();
    }

    bool isSame(const GameObject& object) const {
        return isSame(object.getGameObjectInfo());
    }

    bool isSame(const GameObjectInfo& info) const {
        return info_ == info;
    }

private:
    GameObjectInfo info_;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
