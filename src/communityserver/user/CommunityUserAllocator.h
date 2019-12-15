#pragma once

#include <boost/noncopyable.hpp>

namespace gideon { namespace communityserver {

class CommunityUser;

/**
* @class CommunityUserAllocator
*/
class CommunityUserAllocator : public boost::noncopyable
{
public:
    virtual ~CommunityUserAllocator() {}

    virtual CommunityUser* malloc() = 0;
    virtual void free(CommunityUser* user) = 0;
};

}} // namespace gideon { namespace communityserver {
