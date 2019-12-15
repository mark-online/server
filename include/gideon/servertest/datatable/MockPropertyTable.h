#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/PropertyTable.h>

namespace gideon { namespace servertest {

/**
* @class MockPropertyTable
*/
class GIDEON_SERVER_API MockPropertyTable : public gideon::datatable::PropertyTable
{
public:
    MockPropertyTable() {
        fillProperties();
    }
    virtual ~MockPropertyTable() {}

    virtual const std::wstring* getProperty(const std::wstring& key) const;

private:
    virtual const PropertyMap& getPropertyMap() const {
        return propertyMap_;
    }

    virtual const std::string& getLastError() const {
        static std::string empty;
        return empty;
    }

    virtual bool isLoaded() const {
        return true;
    }

private:
    void fillProperties();

private:
    PropertyMap propertyMap_;
};

}} // namespace gideon { namespace servertest {
