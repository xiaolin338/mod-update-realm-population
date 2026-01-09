#include "mod-update-realm-population.h"
#include "World.h"
#include "WorldSessionMgr.h"
#include "Log.h"
#include "DatabaseEnv.h"
#include "Realm.h"
#include "Config.h"

extern Realm realm;

uint32 GetTotalCharacterCount()
{
    QueryResult result = CharacterDatabase.Query("SELECT COUNT(*) FROM characters");
    if (result)
    {
        Field* fields = result->Fetch();
        return fields[0].Get<uint32>();  // 修正这里
    }
    return 0;
}

void ModUpdateRealmPopulation::OnAfterConfigLoad(bool reload)
{
    // 读取配置
    _enabled = sConfigMgr->GetOption<bool>("ModUpdateRealmPopulation.Enable", true);
    _updateInterval = sConfigMgr->GetOption<uint32>("ModUpdateRealmPopulation.UpdateInterval", 2); // 分钟
    _usePlayerCount = sConfigMgr->GetOption<bool>("ModUpdateRealmPopulation.UsePlayerCount", false);
    if (reload)
    {
        //LOG_INFO("module", "ModUpdateRealmPopulation: Configuration reloaded.");
        //LOG_INFO("module", "ModUpdateRealmPopulation: Enabled: {}, Update Interval: {} minutes", _enabled, _updateInterval);
    }
}

void ModUpdateRealmPopulation::OnStartup()
{
    if (!_enabled)
    {
        LOG_INFO("module", "ModUpdateRealmPopulation: Module is disabled.");
        return;
    }

    // 初始化定时器，使用配置的间隔
    _populationTimer.SetInterval(_updateInterval * MINUTE * IN_MILLISECONDS);
    _populationTimer.Reset();

    LOG_INFO("module", "ModUpdateRealmPopulation: Module is enabled. Population update interval set to {} minutes.", _updateInterval);

    // LOG_INFO("module", "ModUpdateRealmPopulation: Module loaded, population update interval set to {} minutes.", _updateInterval);
}

void ModUpdateRealmPopulation::OnUpdate(uint32 diff)
{
    if (!_enabled)
        return;

    _populationTimer.Update(diff);

    if (_populationTimer.Passed())
    {
        _populationTimer.Reset();

        //LOG_DEBUG("module", "ModUpdateRealmPopulation: Updating realm population...");

        uint32 pLimit = sWorldSessionMgr->GetPlayerAmountLimit();

        //LOG_INFO("module", "ModUpdateRealmPopulation: Player limit: {}", pLimit);

        if (pLimit > 0)
        {
            float population = 0.0f;

             if (_usePlayerCount)
            {
                population = sWorldSessionMgr->GetPlayerCount();
                //population = static_cast<float>(sWorldSessionMgr->GetPlayerCount());
                //LOG_DEBUG("module", "ModUpdateRealmPopulation: Using player count: {}", population);
            }
            else
            {
                population = sWorldSessionMgr->GetActiveSessionCount();
               // population = static_cast<float>(sWorldSessionMgr->GetActiveSessionCount());
                //LOG_DEBUG("module", "ModUpdateRealmPopulation: Using active sessions: {}", population);
            }
           // float population = sWorldSessionMgr->GetActiveSessionCount();
           // population /= pLimit;
           // population *= 2;

            population = (population / pLimit) * 2;
            // 使用 std::min 限制最大值
            population = std::min(population, 2.0f);


           // LOG_INFO("module", "ModUpdateRealmPopulation: Updating realm population: active sessions={}, limit={}, population={}",sWorldSessionMgr->GetActiveSessionCount(), pLimit, population);

            LoginDatabase.Execute("UPDATE realmlist SET population = {} WHERE id = '{}'", population, realm.Id.Realm);

            if (_updateFlag)
            {
                uint32 characterCount = GetTotalCharacterCount();
                uint32 flag = 0;

                if (characterCount == 0)
                {
                    flag = 64;
                    //LOG_DEBUG("module", "ModUpdateRealmPopulation: Character count is 0, setting flag to 64");
                }
                else if (population < 0.5f)
                {
                    flag = 32;
                    //LOG_DEBUG("module", "ModUpdateRealmPopulation: Population ({}) < 0.5, setting flag to 32", population);
                }
                else if (population > 1.9f)
                {
                    flag = 128;
                    //LOG_DEBUG("module", "ModUpdateRealmPopulation: Population ({}) >= 2.0, setting flag to 128", population);
                }
                else
                {
                    //LOG_DEBUG("module", "ModUpdateRealmPopulation: No flag update conditions met. Character count: {}, Population: {}",characterCount, population);
                    return; // 条件不满足时不更新flag
                }

                // 执行flag更新
                LoginDatabase.Execute("UPDATE realmlist SET flag = {} WHERE id = '{}'", flag, realm.Id.Realm);
                //LOG_INFO("module", "ModUpdateRealmPopulation: Updated realm flag to {} for realm id {}", flag, realm.Id.Realm);
            }

        }
    }
}

void Addmod_update_realm_populationScripts()
{
    new ModUpdateRealmPopulation();
}
