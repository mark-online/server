#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/ElementTable.h>

namespace gideon { namespace servertest {

/**
* @class MockElementTable
*/
class GIDEON_SERVER_API MockElementTable : public gideon::datatable::ElementTable
{
public:
	MockElementTable() {
		fillElements();
	}
	virtual ~MockElementTable() {
		destroyElements();
	}

    virtual const gdt::element_t* getElement(ElementCode code) const;

private:
	virtual const ElementMap& getElementMap() const {
		return elementMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
private:
	void fillElement1();
	void fillElement2();
	void fillElement3();
	void fillElement4();
	void fillElement5();
    void fillElement6();

	void fillElements() {
		fillElement1();
		fillElement2();
		fillElement3();
		fillElement4();
		fillElement5();
        fillElement6();
	}

	void destroyElements();

private:
	ElementMap elementMap_;
};

}} // namespace gideon { namespace servertest {