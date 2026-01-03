
#include "mod-update-realm-population.h"
#include "World.h"
#include "WorldSessionMgr.h"
#include "Log.h"
#include "DatabaseEnv.h"
#include "Realm.h"
#include "Config.h"

extern Realm realm;

void ModUpdateRealmPopulation::OnAfterConfigLoad(bool reload)
{
    // 读取配置
    _enabled = sConfigMgr->GetOption<bool>("ModUpdateRealmPopulation.Enable", true);
    _updateInterval = sConfigMgr->GetOption<uint32>("ModUpdateRealmPopulation.UpdateInterval", 2); // 分钟

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
            float population = sWorldSessionMgr->GetActiveSessionCount();
           // population /= pLimit;
           // population *= 2;

            population = (population / pLimit) * 2;
            // 使用 std::min 限制最大值
            population = std::min(population, 2.0f);


           // LOG_INFO("module", "ModUpdateRealmPopulation: Updating realm population: active sessions={}, limit={}, population={}",sWorldSessionMgr->GetActiveSessionCount(), pLimit, population);

            LoginDatabase.Execute("UPDATE realmlist SET population = {} WHERE id = '{}'", population, realm.Id.Realm);
        }
    }
}

void Addmod_update_realm_populationScripts()
{
    new ModUpdateRealmPopulation();
}
