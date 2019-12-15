#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockFragmentTable.h>
#include <gideon/servertest/datatable/DataCodes.h>

namespace gideon { namespace servertest {

const gdt::fragment_t* MockFragmentTable::getFragment(FragmentCode code) const
{
    const FragmentMap::const_iterator pos = fragmentMap_.find(code);
    if (pos != fragmentMap_.end()) {
        return (*pos).second;
    }
    return nullptr; 
}


void MockFragmentTable::fillFragment1()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(shabbyHelmatFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(shabbyHelmatFragmentCode, fragment);
}


void MockFragmentTable::fillFragment2()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(normaHelmatFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(normaHelmatFragmentCode, fragment);
}


void MockFragmentTable::fillFragment3()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(rareHelmatFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(rareHelmatFragmentCode, fragment);
}


void MockFragmentTable::fillFragment4()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(uniqueHelmatFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(uniqueHelmatFragmentCode, fragment);
}


void MockFragmentTable::fillFragment5()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(epicHelmatFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(epicHelmatFragmentCode, fragment);
}


void MockFragmentTable::fillFragment6()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(oneHandSwordFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(oneHandSwordFragmentCode, fragment);
}


void MockFragmentTable::fillFragment7()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(shieldFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(shieldFragmentCode, fragment);
}


void MockFragmentTable::fillFragment8()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(babutaHelmetFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(babutaHelmetFragmentCode, fragment);
}


void MockFragmentTable::fillFragment9()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(lanceFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(lanceFragmentCode, fragment);
}


void MockFragmentTable::fillFragment10()
{
    gdt::fragment_t* fragment = new gdt::fragment_t;
    //fragment->fragment_code(shoesFragmentCode);
    //fragment->looting_delay(5000);
    fragment->stack_count(255);
    fragmentMap_.emplace(shoesFragmentCode, fragment);
}


void MockFragmentTable::destroyFragments()
{
    for (FragmentMap::value_type& value : fragmentMap_) {
        delete value.second;
    }
}

}} // namespace gideon { namespace servertest {
