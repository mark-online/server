#pragma once

#include <gideon/Common.h>
#include <gideon/cs/datatable/FragmentTable.h>

namespace gideon { namespace servertest {

/**
* @class MockFragmentTable
*/
class GIDEON_SERVER_API MockFragmentTable : public gideon::datatable::FragmentTable
{
public:
	MockFragmentTable() {
		fillFragments();
	}

	virtual ~MockFragmentTable() {
		destroyFragments();
	}

	virtual const gdt::fragment_t* getFragment(FragmentCode code) const;

private:
	virtual const FragmentMap& getFragmentMap() const {
		return fragmentMap_;
	}

	virtual const std::string& getLastError() const {
		static std::string empty;
		return empty;
	}

	virtual bool isLoaded() const {
		return true;
	}
private:
	void fillFragment1();
	void fillFragment2();
	void fillFragment3();
	void fillFragment4();
	void fillFragment5();
    void fillFragment6();
    void fillFragment7();
    void fillFragment8();
    void fillFragment9();
    void fillFragment10();

	void fillFragments() {
		fillFragment1();
		fillFragment2();
		fillFragment3();
		fillFragment4();
		fillFragment5();
        fillFragment6();
        fillFragment7();
        fillFragment8();
        fillFragment9();
        fillFragment10();
	}

	void destroyFragments();

private:
	FragmentMap fragmentMap_;
};

}} // namespace gideon { namespace servertest {