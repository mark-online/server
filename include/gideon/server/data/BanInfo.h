#pragma once

#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/Time.h>
#include <sne/core/container/Containers.h>
#include <sne/server/s2s/ServerId.h>

namespace gideon {

/***
 * enum BanMode
 ***/
enum BanMode
{
	bmNone = 0,
	bmAccount,
	bmCharacter,
	bmIpAddress,
};

/**
 * @struct BanInfo
 */
struct BanInfo 
{
	BanMode banMode_;
	AccountId accountId_;
	ObjectId characterId_;
	std::string ip_;	
	sec_t banExpireTime_;
	std::wstring reason_;

	bool isValid() const {
		if (bmAccount == banMode_) {
			return isValidAccountId(accountId_);
		}
		else if (bmCharacter == banMode_) {
			return isValidObjectId(characterId_);
		}
		else if (bmIpAddress == banMode_) {
			return ! ip_.empty();
		}
		return false;
	}

	template <typename Stream>
	void serialize(Stream& stream) {
	    stream & banMode_;
		if (bmAccount == banMode_) {
			stream & accountId_;
		}
		else if (bmCharacter == banMode_) {
			stream & characterId_;
		}
		else if (bmIpAddress == banMode_) {
			stream & ip_;
		}
		else {
			assert(false && "invalid mode");
		}

		stream & banExpireTime_ & reason_;
	}
};

typedef sne::core::List<BanInfo> BanInfos;

} // namespace gideon {


namespace sne { namespace core {

inline OStream& operator<<(OStream& lhs, gideon::BanMode rhs)
{
    lhs << static_cast<uint8_t>(rhs);
    return lhs;
}


inline IStream& operator>>(IStream& lhs, gideon::BanMode& rhs)
{
    uint8_t value;
    lhs >> value;
    rhs = static_cast<gideon::BanMode>(value);
    return lhs;
}

}} // namespace sne { namespace core {