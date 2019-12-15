#pragma once

namespace gideon { namespace zoneserver { namespace go {

/**
 * @class GliderMountable
 * 글라이더를 탈 수 있다
 */
class GliderMountable
{
public:
    virtual ~GliderMountable() {}

public:
    virtual void setGliding(bool isGliding) = 0;

    virtual bool isGliding() const = 0;
};

}}} // namespace gideon { namespace zoneserver { namespace go {
