#pragma once

#include <gideon/cs/shared/data/ObjectInfo.h>

namespace gideon { namespace zoneserver {
class QuestRepository;
}}// namespace gideon { namespace zoneserver {

namespace gideon { namespace zoneserver { namespace go {

class Entity;

/**
 * @class Questable
 * 퀘스트을 할수 있다
 */
class Questable
{
public:
    virtual ~Questable() {}

	virtual QuestRepository& getQuestRepository() = 0;
    virtual const QuestRepository& getQuestRepository() const = 0;

	virtual void killed(go::Entity& victim) = 0;
	virtual ErrorCode canAddItem(QuestCode& questCode, QuestItemCode questItemCode) const = 0;

	
};

}}} // namespace gideon { namespace zoneserver { namespace go {
