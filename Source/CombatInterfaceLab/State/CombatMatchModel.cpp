#include "State/CombatMatchModel.h"

void FCombatMatchModel::ApplyStrike(
    FCombatFighterSnapshot& Attacker,
    FCombatFighterSnapshot& Defender,
    const float Damage,
    const float StaminaCost)
{
    if (Attacker.Stamina < StaminaCost || Defender.Health <= 0.0f)
    {
        return;
    }

    Attacker.Stamina = FMath::Clamp(Attacker.Stamina - FMath::Max(0.0f, StaminaCost), 0.0f, Attacker.MaxStamina);
    Defender.Health = FMath::Clamp(Defender.Health - FMath::Max(0.0f, Damage), 0.0f, Defender.MaxHealth);
    ++Attacker.SignificantStrikes;
}

FCombatMatchSnapshot FCombatMatchModel::Reconcile(
    const FCombatMatchSnapshot& Local,
    const FCombatMatchSnapshot& Authoritative)
{
    return Authoritative.Revision >= Local.Revision ? Authoritative : Local;
}

FCombatPalette FCombatMatchModel::PaletteFor(const ECombatColorVisionMode Mode)
{
    FCombatPalette Palette;
    switch (Mode)
    {
    case ECombatColorVisionMode::Deuteranopia:
        Palette.RedCorner = FLinearColor(0.93f, 0.45f, 0.05f);
        Palette.BlueCorner = FLinearColor(0.1f, 0.52f, 0.95f);
        Palette.Accent = FLinearColor(1.0f, 0.85f, 0.15f);
        break;
    case ECombatColorVisionMode::Protanopia:
        Palette.RedCorner = FLinearColor(0.95f, 0.62f, 0.08f);
        Palette.BlueCorner = FLinearColor(0.0f, 0.65f, 0.88f);
        Palette.Accent = FLinearColor(1.0f, 0.9f, 0.2f);
        break;
    case ECombatColorVisionMode::Tritanopia:
        Palette.RedCorner = FLinearColor(0.95f, 0.16f, 0.35f);
        Palette.BlueCorner = FLinearColor(0.12f, 0.72f, 0.58f);
        Palette.Accent = FLinearColor(0.9f, 0.82f, 0.18f);
        break;
    default:
        break;
    }
    return Palette;
}

int32 FCombatNavigationModel::Move(const int32 CurrentIndex, const int32 ItemCount, const int32 Direction)
{
    if (ItemCount <= 0)
    {
        return INDEX_NONE;
    }

    const int32 SafeCurrent = FMath::Clamp(CurrentIndex, 0, ItemCount - 1);
    return (SafeCurrent + (Direction < 0 ? ItemCount - 1 : 1)) % ItemCount;
}

bool FCombatFlowModel::CanTransition(const ECombatLabScreen From, const ECombatLabScreen To)
{
    if (From == To)
    {
        return true;
    }

    switch (From)
    {
    case ECombatLabScreen::FighterSelect:
        return To == ECombatLabScreen::Settings || To == ECombatLabScreen::MatchHud;
    case ECombatLabScreen::Settings:
        return To == ECombatLabScreen::FighterSelect || To == ECombatLabScreen::Pause;
    case ECombatLabScreen::MatchHud:
        return To == ECombatLabScreen::Pause || To == ECombatLabScreen::Results;
    case ECombatLabScreen::Pause:
        return To == ECombatLabScreen::MatchHud || To == ECombatLabScreen::Settings || To == ECombatLabScreen::FighterSelect || To == ECombatLabScreen::Results;
    case ECombatLabScreen::Results:
        return To == ECombatLabScreen::MatchHud || To == ECombatLabScreen::FighterSelect;
    }
    return false;
}
