#pragma once

#include <gideon/cs/shared/data/ItemDropInfo.h>
#include <gideon/cs/shared/data/NpcInfo.h>
#include <gideon/cs/shared/data/ElementInfo.h>
#include <gideon/cs/shared/data/RecipeInfo.h>
#include <gideon/cs/shared/data/QuestInfo.h>
#include <gideon/cs/shared/data/HarvestInfo.h>
#include <gideon/cs/shared/data/TreasureInfo.h>
#include <gideon/cs/shared/data/AnchorInfo.h>
#include <gideon/cs/shared/data/BuildingInfo.h>
#include <gideon/cs/shared/data/StaticObjectSkillInfo.h>
#include <gideon/cs/shared/data/GliderInfo.h>
#include <gideon/cs/shared/data/VehicleInfo.h>
#include <gideon/cs/shared/data/FactionInfo.h>
#include <gideon/cs/shared/data/FragmentInfo.h>

// TODO: Property 테이블로 이동

namespace gideon { namespace servertest {

const EquipCode defaultOneHandSwordEquipCode = 51446529; //< 초라한 장검
const EquipCode upgradeOneHandSwordEquipCode = 51446530; //< 초라한 장검 + 1

const EquipCode defaultShieldEquipCode = 51512257; //< 초라한 느릅나무 라운드 방패
const EquipCode upgradeShieldEquipCode = 51512258; //< 초라한 느릅나무 라운드 방패 + 1

const EquipCode defaultHelmetEquipCode = 50922529; //< 초라한 철 바부타
const EquipCode upgradeHelmetEquipCode = 50922530; //< 초라한 철 바부타 + 1

const EquipCode otherHelmetEquipCode = 50921473; //< 갈리아식 H형 투구
const EquipCode upgradeOtherHelmetEquipCode = 50921474; //< 갈리아식 H형 투구 + 1

const EquipCode lanceEquipCode = 51578657; //< 미늘창
const EquipCode upgradeLanceEquipCode = 51578658; //< 미늘창 + 1

const EquipCode shoesEquipCode = 51381569; //< 초라한 담비가죽 신발
const EquipCode upgradeShoesEquipCode = 51381570; //< 초라한 담비가죽 신발 + 1

const NpcCode normalNpcCode = 100794369; // 사막전갈
const NpcCode servantNpcCode = 100794372; // 상처입은 미노타우르스
const NpcCode bossNpcCode = 100794373; // 모래 폭풍

const NpcCode questNpcCode = 100794386; //
const NpcCode storeNpcCode = 100728892; //

//const NpcDropCode normalNpcDropCode = 100794369; // 사막전갈
//const NpcDropCode servantNpcDropCode = 100794372; // 상처입은 미노타우르스
//const NpcDropCode bossNpcDropCode = 100794373; // 모래 폭풍

const ElementCode normalNpcElementCode1 = 117440513; // 전갈 꼬리
const ElementCode normalNpcElementCode2 = 117440514; // 전갈 독액
const ElementCode servantNpcElementCode1 = 117440515; // 미노타우르스 뿔
const ElementCode servantNpcElementCode2 = 117440516; // 미노타우르스 피혁
const ElementCode bossNpcElementCode = 117440517; // 모래 원소

const ElementCode extendInventoryElementCode = 117440520; // 인벤 확장아이템

const FragmentCode shabbyHelmatFragmentCode = 167772161;
const FragmentCode normaHelmatFragmentCode = 167772162;
const FragmentCode rareHelmatFragmentCode = 167772163;
const FragmentCode uniqueHelmatFragmentCode = 167772164;
const FragmentCode epicHelmatFragmentCode = 167772165;

const FragmentCode oneHandSwordFragmentCode = 167772209;
const FragmentCode shieldFragmentCode = 167772221;
const FragmentCode babutaHelmetFragmentCode = 167772227;
const FragmentCode lanceFragmentCode = 167772275;
const FragmentCode shoesFragmentCode = 167772245;

const RecipeCode shabbyHelmetRecipeCode = 83886081;
const RecipeCode normalHelmetRecipeCode = 83886082;
const RecipeCode rareHelmetRecipeCode = 83886083;
const RecipeCode uniqueHelmetRecipeCode = 83886084;
const RecipeCode epicHelmetRecipeCode = 83886085;

const QuestCode killScorpionQuestCode = makeQuestCode(1);
const QuestCode obtainScorpionQuestCode = makeQuestCode(2);
const QuestCode transportQuestCode = makeQuestCode(3);
const QuestCode repeatQuestCode = makeQuestCode(4);

const QuestMissionCode scorpionKillMissionCode = makeQuestMissionCode(qmtKill, 1);
const QuestMissionCode scorpionObtainMissionCode = makeQuestMissionCode(qmtObtain, 1);
const QuestMissionCode questItemTransportMissionCode = makeQuestMissionCode(qmtTrasport, 1);

const QuestItemCode transportQuestItemCode = makeQuestItemCode(1);

const HarvestCode harvestCode1 = 251658241;
const HarvestCode harvestCode2 = 251658242;

const TreasureCode treasureCode1 = 268435457;
const TreasureCode treasureCode2 = 268435458;

const AnchorCode shabbyAncorCode = 285212673;

const StaticObjectSkillCode selfAreaHpDamangeSkillCode = 1;
const StaticObjectSkillCode selfDamangeSkillCode = 2;
const StaticObjectSkillCode targetDamangeSkillCode = 3;

const BuildingCode castleWallCode = 318767105;

const FactionCode defaultMonsterFactionCode = 2;

const GliderCode gliderCode1 = 469762049;

const VehicleCode vehicleCode1 = 452984833;

}} // namespace gideon { namespace servertest {