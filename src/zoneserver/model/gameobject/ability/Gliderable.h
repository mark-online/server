#pragma once


namespace gideon { namespace zoneserver { namespace go {

/**
* @class Gliderable
 * .
 */
class Gliderable
{
public:
    virtual ~Gliderable() {}
	
public:
	virtual GliderCode getGliderCode() const = 0;
	virtual uint32_t getGliderDurability() const = 0;

	virtual void addGlider(const GliderInfo& info) = 0;
	virtual void decreaseGliderDurability(uint32_t value) = 0;

	virtual ErrorCode selectGlider(ObjectId selectId) = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
