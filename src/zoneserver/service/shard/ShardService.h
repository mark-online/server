#pragma once

#include <gideon/cs/shared/data/AccountId.h>
#include <gideon/cs/shared/data/UserId.h>
#include <gideon/cs/shared/data/CharacterInfo.h>
#include <gideon/cs/shared/data/ObjectInfo.h>
#include <gideon/cs/shared/data/CreateCharacterInfo.h>
#include <boost/noncopyable.hpp>

namespace gideon {
class CreatureEquipments;
} // namespace gideon {

namespace gideon { namespace zoneserver {

/**
 * @class ShardService
 * Shard 관련 서비스
 */
class ShardService : public boost::noncopyable
{
public:
    /// 로그인 서버로 부터 사용자 정보 요청이 도착하였다.
    static void fullUserInfoRequested(AccountId accountId);

    /// 로그인 서버로 부터 캐릭터 생성 요청이 도착하였다.
    static void createCharacterRequested(const CreateCharacterInfo& createCharacterInfo);

    /// 로그인 서버로 부터 캐릭터 삭제 요청이 도착하였다.
    static void deleteCharacterRequested(AccountId accountId, ObjectId characterId);

    /// 로기인 서버로 부터 닉네임 검사 요청이 도착하였다.
    static void checkDuplicateNicknameRequested(AccountId accountId, const Nickname& nickname);
};

}} // namespace gideon { namespace zoneserver {
