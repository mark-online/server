#include "ZoneServerPCH.h"
#include "GameScriptManager.h"
#include "../Brain.h"
#include "../../model/gameobject/Npc.h"
#include <gideon/cs/datatable/NpcTable.h>
#include <sne/base/utility/Logger.h>
#include <sne/core/container/Containers.h>
#include <sne/core/utility/Hash.h>

namespace gideon { namespace zoneserver { namespace ai {

namespace {

std::string toAnsi(const std::wstring& unicode)
{
    std::string converted;
    converted.reserve(unicode.size());
    for (const wchar_t ch : unicode) {
        converted.push_back(char(ch));
    }
    return converted;
}

/**
 * @struct GameScriptResistry
 */
template <typename ST>
struct GameScriptResistry
{
    typedef sne::core::HashMap<uint32_t, ST*> ScriptMap;
    typedef typename ScriptMap::value_type ScriptMapVauleType;
    typedef typename ScriptMap::iterator ScriptMapIterator;

    static void addScript(ST* script) {
        assert(script != nullptr);

        for (const ScriptMapVauleType& value : scriptMap_) {
            ST* addedScript = value.second;
            if (script == addedScript) {
                SNE_LOG_ERROR("Script '%s' has same memory pointer as '%s'.",
                    script->getName().c_str(), addedScript->getName().c_str());
                return;
            }
        }

        const uint32_t scriptId = getScriptId(script->getName());
        if (getScript(scriptId) != nullptr) {
            SNE_LOG_ERROR("Script '%s' already exists.",
                script->getName().c_str());
            return;
        }

        scriptMap_.emplace(scriptId, script);
    }

    static ST* getScript(uint32_t scriptId) {
        const ScriptMapIterator pos = scriptMap_.find(scriptId);
        if (pos != scriptMap_.end()) {
            return (*pos).second;
        }
        return nullptr;
    }

    static ST* getScript(const std::string& scriptName) {
        return getScript(getScriptId(scriptName));
    }

    static uint32_t getScriptId(const std::string& scriptName) {
        return sne::core::hash(scriptName.c_str());
    }

    static ScriptMap scriptMap_;
};

template <typename ST> sne::core::HashMap<uint32_t, ST*> GameScriptResistry<ST>::scriptMap_;


template <typename ST>
inline ST* getScript(const std::string& scriptName)
{
    return GameScriptResistry<ST>::getScript(scriptName);
}

} // namespace {

// = NpcScript

NpcScript::NpcScript(const char* name) :
    GameScript(name)
{
    GameScriptResistry<NpcScript>::addScript(this);
}

// = GameScriptManager

GameScriptManager& GameScriptManager::instance()
{
    static GameScriptManager s_instance;
    return s_instance;
}


std::unique_ptr<Brain> GameScriptManager::createBrain(go::Npc& owner)
{
    const datatable::NpcTemplate& npcTemplate = owner.getNpcTemplate();

    const gdt::npc_t& npcInfo = npcTemplate.getInfo();
    NpcScript* script = getScript<NpcScript>(toAnsi(npcInfo.ai_script_name().c_str()));
    if (! script) {
        return nullptr;
    }

    return script->createBrain(owner,
        npcInfo.ai_script_param_1(), npcInfo.ai_script_param_2(),
        npcInfo.ai_script_param_3(), npcInfo.ai_script_param_4());
}

// = specialization

template struct GameScriptResistry<NpcScript>;

}}} // namespace gideon { namespace zoneserver { namespace ai {
