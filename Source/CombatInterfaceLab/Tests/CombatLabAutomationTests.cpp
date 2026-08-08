#include "State/CombatMatchModel.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Config/CombatFighterCatalog.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatMatchStrikeRulesTest,
    "CombatInterfaceLab.State.StrikeRules",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatMatchStrikeRulesTest::RunTest(const FString& Parameters)
{
    FCombatFighterSnapshot Attacker;
    FCombatFighterSnapshot Defender;
    FCombatMatchModel::ApplyStrike(Attacker, Defender, 12.0f, 8.0f);
    TestEqual(TEXT("A valid strike damages the defender"), Defender.Health, 88.0f);
    TestEqual(TEXT("A valid strike spends stamina"), Attacker.Stamina, 92.0f);
    TestEqual(TEXT("A valid strike increments the stat"), Attacker.SignificantStrikes, 1);

    Attacker.Stamina = 2.0f;
    FCombatMatchModel::ApplyStrike(Attacker, Defender, 50.0f, 8.0f);
    TestEqual(TEXT("An exhausted fighter cannot deal damage"), Defender.Health, 88.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatResynchronizationRulesTest,
    "CombatInterfaceLab.State.AuthoritativeResynchronization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatResynchronizationRulesTest::RunTest(const FString& Parameters)
{
    FCombatMatchSnapshot Local;
    Local.Revision = 20;
    Local.RedCorner.Health = 40.0f;

    FCombatMatchSnapshot NewServerState;
    NewServerState.Revision = 21;
    NewServerState.RedCorner.Health = 77.0f;
    const FCombatMatchSnapshot Reconciled = FCombatMatchModel::Reconcile(Local, NewServerState);
    TestEqual(TEXT("A newer authoritative revision replaces local values"), Reconciled.RedCorner.Health, 77.0f);

    FCombatMatchSnapshot StalePacket = NewServerState;
    StalePacket.Revision = 19;
    StalePacket.RedCorner.Health = 1.0f;
    const FCombatMatchSnapshot Preserved = FCombatMatchModel::Reconcile(Local, StalePacket);
    TestEqual(TEXT("A stale packet cannot roll state back"), Preserved.Revision, 20);
    TestEqual(TEXT("Local values survive stale packets"), Preserved.RedCorner.Health, 40.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatControllerNavigationTest,
    "CombatInterfaceLab.UI.NavigationWrapsInBothDirections",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatControllerNavigationTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Down advances focus"), FCombatNavigationModel::Move(1, 4, 1), 2);
    TestEqual(TEXT("Down wraps from last to first"), FCombatNavigationModel::Move(3, 4, 1), 0);
    TestEqual(TEXT("Up wraps from first to last"), FCombatNavigationModel::Move(0, 4, -1), 3);
    TestEqual(TEXT("Empty menus have no focus target"), FCombatNavigationModel::Move(0, 0, 1), INDEX_NONE);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatScreenTransitionTest,
    "CombatInterfaceLab.UI.StateTransitions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatScreenTransitionTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Selection can open settings"), FCombatFlowModel::CanTransition(ECombatLabScreen::FighterSelect, ECombatLabScreen::Settings));
    TestTrue(TEXT("Selection can start a match"), FCombatFlowModel::CanTransition(ECombatLabScreen::FighterSelect, ECombatLabScreen::MatchHud));
    TestTrue(TEXT("HUD can pause"), FCombatFlowModel::CanTransition(ECombatLabScreen::MatchHud, ECombatLabScreen::Pause));
    TestTrue(TEXT("HUD can show results"), FCombatFlowModel::CanTransition(ECombatLabScreen::MatchHud, ECombatLabScreen::Results));
    TestTrue(TEXT("Pause can open settings"), FCombatFlowModel::CanTransition(ECombatLabScreen::Pause, ECombatLabScreen::Settings));
    TestFalse(TEXT("Selection cannot jump directly to results"), FCombatFlowModel::CanTransition(ECombatLabScreen::FighterSelect, ECombatLabScreen::Results));
    TestFalse(TEXT("Settings cannot invent a result"), FCombatFlowModel::CanTransition(ECombatLabScreen::Settings, ECombatLabScreen::Results));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatAccessibilityPaletteTest,
    "CombatInterfaceLab.UI.AccessibilityPalettes",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatAccessibilityPaletteTest::RunTest(const FString& Parameters)
{
    const FCombatPalette Standard = FCombatMatchModel::PaletteFor(ECombatColorVisionMode::Standard);
    const FCombatPalette Deuteranopia = FCombatMatchModel::PaletteFor(ECombatColorVisionMode::Deuteranopia);
    const FCombatPalette Protanopia = FCombatMatchModel::PaletteFor(ECombatColorVisionMode::Protanopia);
    const FCombatPalette Tritanopia = FCombatMatchModel::PaletteFor(ECombatColorVisionMode::Tritanopia);
    TestNotEqual(TEXT("Deuteranopia changes the red-corner cue"), Standard.RedCorner, Deuteranopia.RedCorner);
    TestNotEqual(TEXT("Protanopia changes the blue-corner cue"), Standard.BlueCorner, Protanopia.BlueCorner);
    TestNotEqual(TEXT("Tritanopia changes the red-corner cue"), Standard.RedCorner, Tritanopia.RedCorner);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatLuaCatalogTest,
    "CombatInterfaceLab.Config.LuaFighterCatalog",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatLuaCatalogTest::RunTest(const FString& Parameters)
{
    const TArray<FCombatFighterDefinition> Fighters = FCombatFighterCatalog::Load();
    TestTrue(TEXT("At least two valid fighter profiles are available"), Fighters.Num() >= 2);
    if (Fighters.Num() >= 2)
    {
        TestFalse(TEXT("Fighter IDs are present"), Fighters[0].Id.IsNone());
        TestFalse(TEXT("Localized display names are present"), Fighters[0].DisplayName.IsEmpty());
        TestNotEqual(TEXT("Fighter IDs remain unique"), Fighters[0].Id, Fighters[1].Id);
    }
    return true;
}

#endif
