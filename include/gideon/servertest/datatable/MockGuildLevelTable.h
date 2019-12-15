#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/GuildLevelTable.h>

namespace gideon { namespace servertest {

/**
* @class MockGuildLevelTable
*/
class GIDEON_SERVER_API MockGuildLevelTable : public gideon::datatable::GuildLevelTable
{
public:
    const uint32_t guildMemberCount = 10;

public:
    MockGuildLevelTable();

private:
    virtual GuildLevel getGuildLevel(GuildExp exp) const {
        exp;
        return glStart;
    }

    virtual GuildExp getGuildLevelExp(GuildLevel level) const {
        level;
        return 100;
    }

    virtual uint32_t getGuildMemberCount(GuildLevel level) const {
        level;
        return guildMemberCount;
    }

    virtual const GuildExps& getGuildExps() const {
        return guildExps_;
    }

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
    GuildExps guildExps_;
};

}} // namespace gideon { namespace servertest {
