#pragma once

#include "CoreMinimal.h"
#include "State/CombatLabTypes.h"

/** Deterministic, UObject-free rules used by runtime state and fast automation tests. */
struct FCombatMatchModel
{
    static void ApplyStrike(FCombatFighterSnapshot& Attacker, FCombatFighterSnapshot& Defender, float Damage, float StaminaCost);
    static FCombatMatchSnapshot Reconcile(const FCombatMatchSnapshot& Local, const FCombatMatchSnapshot& Authoritative);
    static FCombatPalette PaletteFor(ECombatColorVisionMode Mode);
};

struct FCombatNavigationModel
{
    static int32 Move(int32 CurrentIndex, int32 ItemCount, int32 Direction);
};

struct FCombatFlowModel
{
    static bool CanTransition(ECombatLabScreen From, ECombatLabScreen To);
};
