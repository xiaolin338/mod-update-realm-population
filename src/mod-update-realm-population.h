#ifndef MOD_UPDATE_REALM_POPULATION_H
#define MOD_UPDATE_REALM_POPULATION_H

#include "ScriptMgr.h"

class ModUpdateRealmPopulation : public WorldScript
{
public:
    ModUpdateRealmPopulation() : WorldScript("mod_update_realm_population") { }

    void OnAfterConfigLoad(bool reload) override;
    void OnStartup() override;
    void OnUpdate(uint32 diff) override;

private:
    IntervalTimer _populationTimer;
    uint32 _updateInterval;
    bool _enabled;
    bool _usePlayerCount;
};

#endif // MOD_UPDATE_REALM_POPULATION_H
