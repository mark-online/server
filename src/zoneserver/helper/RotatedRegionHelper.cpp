#include "ZoneServerPCH.h"
#include "RotatedRegionHelper.h"
#include <gideon/3d/3d.h>
#include <esut/Random.h>

namespace gideon { namespace zoneserver {

RotatedRegionRect::RotatedRegionRect(const Vector2& minPos, const Vector2& maxPos, Heading heading)
{
    computeRotatedRect(minPos, maxPos, heading);
}

bool RotatedRegionRect::isInRect(const Vector2& pos) const
{
    // 현재 직사각형의 원점으로 이동
    Vector2 p = pos - center_;

    // 회전
    p = Vector2( p.x * cosf(toRadian(heading_)) - p.y * sinf(toRadian(heading_)) , p.x * sinf(toRadian(heading_)) + p.y * cosf(toRadian(heading_)));

    if( (maxPos_ - center_).x < p.x ) return false;
    if( (maxPos_ - center_).y < p.y ) return false;
    if( (minPos_ - center_).x > p.x) return false;
    if( (minPos_ - center_).y > p.y) return false;

    return true;
}

Vector2 RotatedRegionRect::getRandomPoint() const
{
    Vector2 randomPoint(0.0f, 0.0f);
    
    // random ( 원점으로 이동 )
    randomPoint.x = esut::random(minPos_.x - center_.x, maxPos_.x - center_.x);
    randomPoint.y = esut::random(minPos_.y - center_.y, maxPos_.y - center_.y);

    // 역 회전
    Vector2 transitionPoint = randomPoint - Vector2( randomPoint.x * cosf(toRadian(heading_)) - randomPoint.y * sinf(toRadian(heading_)) , randomPoint.x * sinf(toRadian(heading_)) + randomPoint.y * cosf(toRadian(heading_)));
    randomPoint -= transitionPoint;

    // 원위치
    randomPoint += center_;

    return randomPoint;
}

// TODO: 최적화!!!!!!!
Vector2 RotatedRegionRect::getClampPos(const Vector2& pos, const Vector2& des) const
{
    Vector2 posToTransformCenter = pos - center_;
    Vector2 desToTransformCenter = des - center_;

    float32_t resultPosX = 0.0f;
    float32_t resultPosY = 0.0f;

    if(maxPos_.x < des.x)
        resultPosX = maxPos_.x;
    else if(minPos_.x > des.x)
        resultPosX = minPos_.x;
    else
    {
        // x = ay + c ( x 축을 중심으로 ) 
        float32_t a = ( pos.x - des.x ) / ( pos.y - des.y );
        float32_t b = pos.x - a * pos.y;

        if(maxPos_.y < des.y)
            resultPosX = a * maxPos_.y + b;
        else
            resultPosX = a * minPos_.y + b;
    }

    if(maxPos_.y < des.y)
        resultPosY = maxPos_.y;
    else if(minPos_.y > des.y)
        resultPosY = minPos_.y;
    else
    {
        // y = ax + c ( y 축을 중심으로 )
        float32_t a = ( pos.y - des.y ) / ( pos.x - des.x );
        float32_t b = pos.y - a * pos.x;

        if(maxPos_.x < des.x)
            resultPosY = a * maxPos_.x + b;
        else
            resultPosY = a * minPos_.x + b;
    }

    Vector2 resultPos(resultPosX, resultPosY);

    // 역 회전
    Vector2 transitionPoint = resultPos - Vector2( resultPos.x * cosf(toRadian(heading_)) - resultPos.y * sinf(toRadian(heading_)),
        resultPos.x * sinf(toRadian(heading_)) + resultPos.y * cosf(toRadian(heading_)));
    resultPos -= transitionPoint;

    // 원위치
    resultPos += center_;

    return resultPos;
}

void RotatedRegionRect::computeRotatedRect(const Vector2& minPos, const Vector2& maxPos, Heading heading)
{
    heading_ = heading;

    const float32_t len = glm::length(maxPos - minPos);

    center_ = Vector2(maxPos.x - (len / 2.0f), maxPos.y - (len / 2.0f));

    maxPos_ = maxPos;
    minPos_ = minPos;
}

}} // namespace gideon { namespace zoneserver {
