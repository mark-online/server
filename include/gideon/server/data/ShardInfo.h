#pragma once

#include <gideon/cs/shared/data/ServerInfo.h>
#include <sne/server/s2s/ServerId.h>

namespace gideon {

typedef sne::srpc::RHashMap<ZoneId, sne::server::ServerId> ZoneServerIdMap;


/**
 * @struct FullShardInfo
 */
struct FullShardInfo : public ShardInfo
{
	ZoneServerInfoMap zoneServerInfoMap_;
    ZoneServerIdMap zoneServerIdMap_; ///< FYI: serializing하지 않음!

    FullShardInfo() {}

    bool isValid() const {
        return ShardInfo::isValid() && (! zoneServerInfoMap_.empty());
    }

    ZoneServerInfo* getZoneServerInfo(ZoneId zoneId) {
        const ZoneServerInfoMap::iterator pos = zoneServerInfoMap_.find(zoneId);
        if (pos != zoneServerInfoMap_.end()) {
            return &(*pos).second;
        }
        return nullptr;
    }

    const ZoneServerInfo* getZoneServerInfo(ZoneId zoneId) const {
        const ZoneServerInfoMap::const_iterator pos = zoneServerInfoMap_.find(zoneId);
        if (pos != zoneServerInfoMap_.end()) {
            return &(*pos).second;
        }
        return nullptr;
    }

    sne::server::ServerId getServerId(ZoneId zoneId) const {
        const ZoneServerIdMap::const_iterator pos = zoneServerIdMap_.find(zoneId);
        if (pos != zoneServerIdMap_.end()) {
            return (*pos).second;
        }
        return sne::server::ServerId::invalid;
    }

    sne::server::ServerId getServerIdOfFirstZoneServer() const {
        for (const ZoneServerInfoMap::value_type& value : zoneServerInfoMap_) {
            const ZoneId zoneId = value.first;
            const ZoneServerInfo& zoneInfo = value.second;
            if (zoneInfo.isFirstZone_ && zoneInfo.isEnabled_) {
                return sne::core::search_map(zoneServerIdMap_, zoneId,
                    sne::server::ServerId::invalid);
            }
        }
        return sne::server::ServerId::invalid;
    }

    bool isAnyZoneServerEnabled() const {
        if (zoneServerInfoMap_.empty()) {
            return false;
        }

        for (const ZoneServerInfoMap::value_type& value : zoneServerInfoMap_) {
            const ZoneServerInfo& zoneInfo = value.second;
            if (! zoneInfo.isEnabled_) {
                return false;
            }
        }

        return true;
    }

	template <typename Stream>
	void serialize(Stream& stream) {
        ShardInfo::serialize(stream);
		stream & zoneServerInfoMap_;
	}
};


typedef sne::srpc::RHashMap<ShardId, FullShardInfo> FullShardInfoMap;

} // namespace gideon {
