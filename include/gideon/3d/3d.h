#pragma once

#include "glm.h"
#include <gideon/cs/shared/data/Coordinate.h>
#include <esut/Random.h>
#include <boost/static_assert.hpp>
#include <cmath>

// 참고: 타입 변환은 as, 값으로의 변환은 to prefix 사용

namespace gideon {

using Vector2 = glm::vec2;
using Vector3 = glm::vec3;

BOOST_STATIC_ASSERT(sizeof(Vector3) == sizeof(Position));

const float32_t zeroEpsilon = 0.00002f;


inline float32_t square(float32_t value)
{
    return std::pow(value, 2);
}


inline float32_t toRadians(float32_t degree)
{
    return glm::radians(degree);
}


inline float32_t toDegrees(float32_t radian)
{
    return glm::degrees(radian);
}


inline float32_t squaredLength(const Vector2& v)
{
    return glm::length2(v);
}


inline float32_t squaredLength(const Vector3& v)
{
    return glm::length2(v);
}


inline bool isZero(const Vector2& v)
{
    return squaredLength(v) < zeroEpsilon;
}


inline bool isZero(const Vector3& v)
{
    return squaredLength(v) < zeroEpsilon;
}


inline const Vector3& asVector3(const Position& p)
{
    return reinterpret_cast<const Vector3&>(p);
}


inline Vector3& asVector3(Position& p)
{
    return reinterpret_cast<Vector3&>(p);
}


inline const Vector2& asVector2(const Position& p)
{
    return reinterpret_cast<const Vector2&>(p);
}


inline Vector2& asVector2(Position& p)
{
    return reinterpret_cast<Vector2&>(p);
}


inline const Position& asPosition(const Vector3& v)
{
    return reinterpret_cast<const Position&>(v);
}


inline Position& asPosition(Vector3& v)
{
    return reinterpret_cast<Position&>(v);
}


inline Vector2& normalize(Vector2& vector, float32_t length)
{
    //vector /= length;
    //return vector;

    if (length < 0.0000001f) {
        vector = Vector2();
    }
    else if ((length < 1.00001f) && (length > 0.99999f)) {
        ;
    }
    else {
        vector *= (1.0f / length);
    }
    return vector;
}


inline Vector2 normalizeTo(const Vector2& vector, float32_t length)
{
    if (length < 0.0000001f) {
        return Vector2();
    }
    else if ((length < 1.00001f) && (length > 0.99999f)) {
        return vector;
    }
    return vector * (1.0f / length);
}


inline void truncate(Vector2& v, float32_t max)
{
    const float32_t vLength = glm::length(v);
    if (vLength > max) {
        normalize(v, vLength);
        v *= max;
    } 
}

/// 2D(x,y) 상의 방향 벡터를 구한다
inline Vector2 getDirection(const Position& target, const Position& source)
{
    return glm::normalize(asVector2(target) - asVector2(source));
}


/// source에서 destination까지의 2D 좌표(x, y) 상의 길이를 구한다
inline float32_t get2dDistanceSqTo(const Position& source, const Position& destination)
{
    return squaredLength(asVector2(destination) - asVector2(source));
}


/// source에서 destination까지의 2D 좌표(x, y) 상의 길이를 구한다
inline float32_t get2dDistanceTo(const Position& source, const Position& destination)
{
    return sqrtf(get2dDistanceSqTo(source, destination));
}


inline float32_t get3dDistanceTo(const Position& source, const Position& destination)
{
    return glm::length(asVector3(destination) - asVector3(source));
}


/// Yaw 회전각을 구한다
/// @param direction 방향 벡터(단위 벡터일 필요 없음)
inline float32_t getYaw(const Vector2& direction)
{
    return atan2f(direction.x, direction.y);
}


/// Yaw 회전각을 구한다
inline float32_t getYaw(const Position& p1, const Position& p2)
{
    return getYaw(asVector2(p1) - asVector2(p2));
}


/// Yaw 회전각을 구한다
/// @param direction 방향 벡터(단위 벡터일 필요 없음)
inline Heading getHeading(const Vector2& direction)
{
    return toHeading(toDegrees(getYaw(direction)));
}


/// Yaw 회전각을 구한다
inline Heading getHeading(const Position& p1, const Position& p2)
{
    return toHeading(toDegrees(getYaw(p1, p2)));
}


/// heading을 이용하여 방향 벡터를 구한다
/// @pre isValidHeading(heading) == true
inline Vector2 getDirection(Heading heading)
{
    static const float32_t directionTable[maxHeading + 1][2] =
    {
        { 0.0000f, 1.0000f }, { 0.0349f, 0.9994f }, { 0.0698f, 0.9976f },
        { 0.1045f, 0.9945f }, { 0.1392f, 0.9903f }, { 0.1736f, 0.9848f },
        { 0.2079f, 0.9781f }, { 0.2419f, 0.9703f }, { 0.2756f, 0.9613f },
        { 0.3090f, 0.9511f }, { 0.3420f, 0.9397f }, { 0.3746f, 0.9272f },
        { 0.4067f, 0.9135f }, { 0.4384f, 0.8988f }, { 0.4695f, 0.8829f },
        { 0.5000f, 0.8660f }, { 0.5299f, 0.8480f }, { 0.5592f, 0.8290f },
        { 0.5878f, 0.8090f }, { 0.6157f, 0.7880f }, { 0.6428f, 0.7660f },
        { 0.6691f, 0.7431f }, { 0.6947f, 0.7193f }, { 0.7193f, 0.6947f },
        { 0.7431f, 0.6691f }, { 0.7660f, 0.6428f }, { 0.7880f, 0.6157f },
        { 0.8090f, 0.5878f }, { 0.8290f, 0.5592f }, { 0.8480f, 0.5299f },
        { 0.8660f, 0.5000f }, { 0.8829f, 0.4695f }, { 0.8988f, 0.4384f },
        { 0.9135f, 0.4067f }, { 0.9272f, 0.3746f }, { 0.9397f, 0.3420f },
        { 0.9511f, 0.3090f }, { 0.9613f, 0.2756f }, { 0.9703f, 0.2419f },
        { 0.9781f, 0.2079f }, { 0.9848f, 0.1736f }, { 0.9903f, 0.1392f },
        { 0.9945f, 0.1045f }, { 0.9976f, 0.0698f }, { 0.9994f, 0.0349f },
        { 1.0000f, -0.0000f }, { 0.9994f, -0.0349f }, { 0.9976f, -0.0698f },
        { 0.9945f, -0.1045f }, { 0.9903f, -0.1392f }, { 0.9848f, -0.1736f },
        { 0.9781f, -0.2079f }, { 0.9703f, -0.2419f }, { 0.9613f, -0.2756f },
        { 0.9511f, -0.3090f }, { 0.9397f, -0.3420f }, { 0.9272f, -0.3746f },
        { 0.9135f, -0.4067f }, { 0.8988f, -0.4384f }, { 0.8829f, -0.4695f },
        { 0.8660f, -0.5000f }, { 0.8480f, -0.5299f }, { 0.8290f, -0.5592f },
        { 0.8090f, -0.5878f }, { 0.7880f, -0.6157f }, { 0.7660f, -0.6428f },
        { 0.7431f, -0.6691f }, { 0.7193f, -0.6947f }, { 0.6947f, -0.7193f },
        { 0.6691f, -0.7431f }, { 0.6428f, -0.7660f }, { 0.6157f, -0.7880f },
        { 0.5878f, -0.8090f }, { 0.5592f, -0.8290f }, { 0.5299f, -0.8480f },
        { 0.5000f, -0.8660f }, { 0.4695f, -0.8829f }, { 0.4384f, -0.8988f },
        { 0.4067f, -0.9135f }, { 0.3746f, -0.9272f }, { 0.3420f, -0.9397f },
        { 0.3090f, -0.9511f }, { 0.2756f, -0.9613f }, { 0.2419f, -0.9703f },
        { 0.2079f, -0.9781f }, { 0.1736f, -0.9848f }, { 0.1392f, -0.9903f },
        { 0.1045f, -0.9945f }, { 0.0698f, -0.9976f }, { 0.0349f, -0.9994f },
        { -0.0000f, -1.0000f }, { -0.0349f, -0.9994f }, { -0.0698f, -0.9976f },
        { -0.1045f, -0.9945f }, { -0.1392f, -0.9903f }, { -0.1736f, -0.9848f },
        { -0.2079f, -0.9781f }, { -0.2419f, -0.9703f }, { -0.2756f, -0.9613f },
        { -0.3090f, -0.9511f }, { -0.3420f, -0.9397f }, { -0.3746f, -0.9272f },
        { -0.4067f, -0.9135f }, { -0.4384f, -0.8988f }, { -0.4695f, -0.8829f },
        { -0.5000f, -0.8660f }, { -0.5299f, -0.8480f }, { -0.5592f, -0.8290f },
        { -0.5878f, -0.8090f }, { -0.6157f, -0.7880f }, { -0.6428f, -0.7660f },
        { -0.6691f, -0.7431f }, { -0.6947f, -0.7193f }, { -0.7193f, -0.6947f },
        { -0.7431f, -0.6691f }, { -0.7660f, -0.6428f }, { -0.7880f, -0.6157f },
        { -0.8090f, -0.5878f }, { -0.8290f, -0.5592f }, { -0.8480f, -0.5299f },
        { -0.8660f, -0.5000f }, { -0.8829f, -0.4695f }, { -0.8988f, -0.4384f },
        { -0.9135f, -0.4067f }, { -0.9272f, -0.3746f }, { -0.9397f, -0.3420f },
        { -0.9511f, -0.3090f }, { -0.9613f, -0.2756f }, { -0.9703f, -0.2419f },
        { -0.9781f, -0.2079f }, { -0.9848f, -0.1736f }, { -0.9903f, -0.1392f },
        { -0.9945f, -0.1045f }, { -0.9976f, -0.0698f }, { -0.9994f, -0.0349f },
        { -1.0000f, 0.0000f }, { -0.9994f, 0.0349f }, { -0.9976f, 0.0698f },
        { -0.9945f, 0.1045f }, { -0.9903f, 0.1392f }, { -0.9848f, 0.1736f },
        { -0.9781f, 0.2079f }, { -0.9703f, 0.2419f }, { -0.9613f, 0.2756f },
        { -0.9511f, 0.3090f }, { -0.9397f, 0.3420f }, { -0.9272f, 0.3746f },
        { -0.9135f, 0.4067f }, { -0.8988f, 0.4384f }, { -0.8829f, 0.4695f },
        { -0.8660f, 0.5000f }, { -0.8480f, 0.5299f }, { -0.8290f, 0.5592f },
        { -0.8090f, 0.5878f }, { -0.7880f, 0.6157f }, { -0.7660f, 0.6428f },
        { -0.7431f, 0.6691f }, { -0.7193f, 0.6947f }, { -0.6947f, 0.7193f },
        { -0.6691f, 0.7431f }, { -0.6428f, 0.7660f }, { -0.6157f, 0.7880f },
        { -0.5878f, 0.8090f }, { -0.5592f, 0.8290f }, { -0.5299f, 0.8480f },
        { -0.5000f, 0.8660f }, { -0.4695f, 0.8829f }, { -0.4384f, 0.8988f },
        { -0.4067f, 0.9135f }, { -0.3746f, 0.9272f }, { -0.3420f, 0.9397f },
        { -0.3090f, 0.9511f }, { -0.2756f, 0.9613f }, { -0.2419f, 0.9703f },
        { -0.2079f, 0.9781f }, { -0.1736f, 0.9848f }, { -0.1392f, 0.9903f },
        { -0.1045f, 0.9945f }, { -0.0698f, 0.9976f }, { -0.0349f, 0.9994f },
        { 0.0000f, 1.0000f }
    };

    assert(isValidHeading(heading));
    return Vector2(directionTable[heading][0], directionTable[heading][1]);

    //const float32_t radian = toRadian(heading);
    //return Vector2(sinf(radian), cosf(radian)).directionOrZero();
}


/// 바라 보는 방향의 앞에 있는가?
/// @pre dirTarget = unit vector
inline bool isInForward(const Vector2& dirTarget, const Vector2& heading)
{
    //assert(glm::length(dirTarget) <= 1.0f);
    return glm::dot(dirTarget, heading) >= 0.0f;
}


/// 바라 보는 방향의 앞에 있는가?
inline bool isInForward(const Vector2& dirTarget, Heading heading)
{
    return isInForward(dirTarget, getDirection(heading));
}


inline void clamp2d(Position& p, const Position& low, const Position& high)
{
    p.x_ = glm::clamp(p.x_, low.x_, high.x_);
    p.y_ = glm::clamp(p.y_, low.y_, high.y_);
    //destination_.z_ = ;
}


inline void keepAway(Position& position, float32_t delta)
{
    const Vector2 direction = getDirection(esut::random() % maxHeading);
    position.x_ += direction.x * delta;
    position.y_ += direction.y * delta;
}

} // namespace gideon {
