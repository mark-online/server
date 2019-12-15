#include "ServerTestPCH.h"
#include <gideon/servertest/datatable/MockAnchorCoordinates.h>
#include <boost/foreach.hpp>

namespace gideon { namespace servertest {

const gdt::anchor_base_coordinates_t* MockAnchorCoordinates::getAnchorCoordinates(AnchorCode code) const
{
    BOOST_FOREACH(const gdt::anchor_base_coordinates_t* anchor, coordinatess_) {
        if (anchor->anchor_code() == code) {
            return anchor;
        }
    }
    
    return nullptr;
}



void MockAnchorCoordinates::destroyCoordinates()
{
    BOOST_FOREACH(const gdt::anchor_base_coordinates_t* anchor, coordinatess_) {
        delete anchor;
    }
}

}} // namespace gideon { namespace servertest {