#pragma once

#include "CoreMinimal.h"
#include "State/CombatLabTypes.h"

/** Loads the deliberately small, data-only fighter{} subset used by Fighters.lua. */
class FCombatFighterCatalog
{
public:
    static TArray<FCombatFighterDefinition> Load();
    static FText ResolveDisplayName(FName NameKey, const FString& Fallback);

private:
    static TArray<FCombatFighterDefinition> Defaults();
};
