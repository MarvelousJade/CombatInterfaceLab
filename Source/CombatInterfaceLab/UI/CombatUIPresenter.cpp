#include "UI/CombatUIPresenter.h"

#include "Config/CombatFighterCatalog.h"
#include "Player/CombatLabPlayerController.h"
#include "State/CombatMatchGameState.h"
#include "State/CombatMatchModel.h"

#define LOCTEXT_NAMESPACE "CombatUIPresenter"

void UCombatUIPresenter::Initialize(ACombatLabPlayerController* InOwner)
{
    Owner = InOwner;
    Fighters = FCombatFighterCatalog::Load();
    if (Owner)
    {
        MatchState = Owner->GetWorld() ? Owner->GetWorld()->GetGameState<ACombatMatchGameState>() : nullptr;
    }
    if (MatchState)
    {
        MatchState->OnSnapshotChanged.AddUObject(this, &UCombatUIPresenter::HandleSnapshot);
    }
}

void UCombatUIPresenter::BeginDestroy()
{
    if (MatchState)
    {
        MatchState->OnSnapshotChanged.RemoveAll(this);
    }
    Super::BeginDestroy();
}

const FCombatMatchSnapshot& UCombatUIPresenter::GetSnapshot() const
{
    return MatchState ? MatchState->GetSnapshot() : EmptySnapshot;
}

FCombatPalette UCombatUIPresenter::GetPalette() const
{
    return FCombatMatchModel::PaletteFor(Accessibility.ColorVisionMode);
}

FText UCombatUIPresenter::FighterName(const FName FighterId) const
{
    if (const FCombatFighterDefinition* Found = Fighters.FindByPredicate(
        [FighterId](const FCombatFighterDefinition& Fighter) { return Fighter.Id == FighterId; }))
    {
        return Found->DisplayName;
    }
    return LOCTEXT("UnknownFighter", "UNKNOWN FIGHTER");
}

void UCombatUIPresenter::SelectFighter(const int32 Index)
{
    if (!Fighters.IsValidIndex(Index))
    {
        return;
    }
    SelectedFighterIndex = Index;
    OnFighterSelectionChanged.Broadcast(SelectedFighterIndex);
}

void UCombatUIPresenter::StartMatch()
{
    if (!Owner || !Fighters.IsValidIndex(SelectedFighterIndex))
    {
        return;
    }
    Owner->RequestStartMatch(Fighters[SelectedFighterIndex].Id);
    TransitionTo(ECombatLabScreen::MatchHud);
    Announce(LOCTEXT("RoundOneSubtitle", "Announcer: Round one. Fight!"));
}

void UCombatUIPresenter::OpenSettings()
{
    SettingsReturnScreen = CurrentScreen == ECombatLabScreen::Pause
        ? ECombatLabScreen::Pause
        : ECombatLabScreen::FighterSelect;
    TransitionTo(ECombatLabScreen::Settings);
}

void UCombatUIPresenter::CloseSettings()
{
    TransitionTo(SettingsReturnScreen);
}

void UCombatUIPresenter::PauseMatch()
{
    if (CurrentScreen == ECombatLabScreen::MatchHud)
    {
        Owner->SetDemoPaused(true);
        TransitionTo(ECombatLabScreen::Pause);
    }
}

void UCombatUIPresenter::ResumeMatch()
{
    if (CurrentScreen == ECombatLabScreen::Pause)
    {
        Owner->SetDemoPaused(false);
        TransitionTo(ECombatLabScreen::MatchHud);
    }
}

void UCombatUIPresenter::ReturnToSelection()
{
    if (Owner)
    {
        Owner->SetDemoPaused(false);
    }
    TransitionTo(ECombatLabScreen::FighterSelect, true);
}

void UCombatUIPresenter::Rematch()
{
    if (!Owner || !Fighters.IsValidIndex(SelectedFighterIndex))
    {
        return;
    }
    Owner->RequestStartMatch(Fighters[SelectedFighterIndex].Id);
    TransitionTo(ECombatLabScreen::MatchHud, true);
    Announce(LOCTEXT("RematchSubtitle", "Announcer: The rematch is underway."));
}

void UCombatUIPresenter::Strike()
{
    if (Owner && CurrentScreen == ECombatLabScreen::MatchHud)
    {
        Owner->RequestPlayerStrike();
        Announce(LOCTEXT("StrikeSubtitle", "Corner: Clean connection! Keep the pressure on."));
    }
}

void UCombatUIPresenter::RequestConnectionSimulation()
{
    if (Owner)
    {
        Owner->SimulateConnectionInterruption();
    }
}

void UCombatUIPresenter::CycleColorVisionMode()
{
    const int32 Count = static_cast<int32>(ECombatColorVisionMode::Tritanopia) + 1;
    Accessibility.ColorVisionMode = static_cast<ECombatColorVisionMode>(
        (static_cast<int32>(Accessibility.ColorVisionMode) + 1) % Count);
    OnAccessibilityChanged.Broadcast(Accessibility);
}

void UCombatUIPresenter::ToggleSubtitles()
{
    Accessibility.bSubtitlesEnabled = !Accessibility.bSubtitlesEnabled;
    OnAccessibilityChanged.Broadcast(Accessibility);
}

void UCombatUIPresenter::ToggleReducedMotion()
{
    Accessibility.bReducedMotion = !Accessibility.bReducedMotion;
    OnAccessibilityChanged.Broadcast(Accessibility);
}

void UCombatUIPresenter::CycleInterfaceScale()
{
    if (Accessibility.InterfaceScale < 0.95f)
    {
        Accessibility.InterfaceScale = 1.0f;
    }
    else if (Accessibility.InterfaceScale < 1.05f)
    {
        Accessibility.InterfaceScale = 1.1f;
    }
    else
    {
        Accessibility.InterfaceScale = 0.9f;
    }
    OnAccessibilityChanged.Broadcast(Accessibility);
}

void UCombatUIPresenter::SetConnectionStatus(const ECombatConnectionStatus NewStatus)
{
    if (ConnectionStatus == NewStatus)
    {
        return;
    }
    ConnectionStatus = NewStatus;
    OnConnectionChanged.Broadcast(ConnectionStatus);
}

void UCombatUIPresenter::ApplyResynchronizedSnapshot(const FCombatMatchSnapshot& Snapshot)
{
    if (MatchState)
    {
        MatchState->ApplyResynchronizedSnapshot(Snapshot);
    }
    SetConnectionStatus(ECombatConnectionStatus::Connected);
    Announce(LOCTEXT("ResyncSubtitle", "System: Connection restored. Match state synchronized."));
}

void UCombatUIPresenter::ShowScreenForAutomation(const ECombatLabScreen Screen)
{
#if WITH_AUTOMATION_TESTS
    TransitionTo(Screen, true);
#endif
}

void UCombatUIPresenter::TransitionTo(const ECombatLabScreen NewScreen, const bool bForce)
{
    if (!bForce && !FCombatFlowModel::CanTransition(CurrentScreen, NewScreen))
    {
        return;
    }
    CurrentScreen = NewScreen;
    OnScreenChanged.Broadcast(CurrentScreen);
}

void UCombatUIPresenter::HandleSnapshot(const FCombatMatchSnapshot& Snapshot)
{
    OnSnapshotChanged.Broadcast(Snapshot);
    if (Snapshot.Phase == ECombatMatchPhase::Complete && CurrentScreen != ECombatLabScreen::Results)
    {
        if (Owner)
        {
            Owner->SetDemoPaused(false);
        }
        TransitionTo(ECombatLabScreen::Results, true);
        Announce(LOCTEXT("WinnerSubtitle", "Announcer: The contest is over."));
    }
}

void UCombatUIPresenter::Announce(const FText& Text)
{
    if (Accessibility.bSubtitlesEnabled)
    {
        OnSubtitleRequested.Broadcast(Text);
    }
}

#undef LOCTEXT_NAMESPACE
