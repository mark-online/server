#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/AnchorTable.h>

namespace gideon { namespace servertest {

/**
* @class MockAnchorTable
*/
class GIDEON_SERVER_API MockAnchorTable : public gideon::datatable::AnchorTable
{
public:
	MockAnchorTable();
	virtual ~MockAnchorTable() {
		destroyAnchors();
	}
	virtual const gideon::datatable::AnchorTemplate* getAnchorTemplate(AnchorCode code) const;

private:
	void fillAnchor();
	void destroyAnchors();

	virtual const AnchorMap& getAnchorMap() const {
		return anchorMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
	
private:
	AnchorMap anchorMap_;
};

}} // namespace gideon { namespace servertest {