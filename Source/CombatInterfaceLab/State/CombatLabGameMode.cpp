#include "State/CombatLabGameMode.h"

#include "Player/CombatLabPlayerController.h"
#include "State/CombatMatchGameState.h"
#include "TimerManager.h"

ACombatLabGameMode::ACombatLabGameMode()
{
    GameStateClass = ACombatMatchGameState::StaticClass();
    PlayerControllerClass = ACombatLabPlayerController::StaticClass();
    DefaultPawnClass = nullptr;
    HUDClass = nullptr;
    bPauseable = true;
}

void ACombatLabGameMode::BeginPlay()
{
    Super::BeginPlay();
    GetWorldTimerManager().SetTimer(ClockTimer, this, &ACombatLabGameMode::HandleClockTick, 1.0f, true);
    GetWorldTimerManager().SetTimer(SimulationTimer, this, &ACombatLabGameMode::HandleSimulationTick, 0.75f, true);
}

void ACombatLabGameMode::RequestStartDemoMatch(const FName SelectedFighter)
{
    ACombatMatchGameState* State = GetGameState<ACombatMatchGameState>();
    if (!State)
    {
        return;
    }

    const FName Red = SelectedFighter.IsNone() ? FName(TEXT("marisol_vega")) : SelectedFighter;
    const FName Blue = Red == TEXT("amara_okafor") ? FName(TEXT("sofia_kovac")) : FName(TEXT("amara_okafor"));
    SimulationStep = 0;
    State->InitializeMatch(Red, Blue, RoundSeconds, TotalRounds);
}

void ACombatLabGameMode::HandlePlayerStrike()
{
    if (ACombatMatchGameState* State = GetGameState<ACombatMatchGameState>())
    {
        State->ApplyStrike(true, 7.0f, 8.0f);
        EvaluateMatchEnd();
    }
}

void ACombatLabGameMode::FinishDemoMatch()
{
    if (ACombatMatchGameState* State = GetGameState<ACombatMatchGameState>())
    {
        State->CompleteMatch(State->GetSnapshot().RedCorner.FighterId);
    }
}

void ACombatLabGameMode::HandleClockTick()
{
    ACombatMatchGameState* State = GetGameState<ACombatMatchGameState>();
    if (!State || State->GetSnapshot().Phase != ECombatMatchPhase::InProgress)
    {
        return;
    }

    State->AdvanceClock();
    const FCombatMatchSnapshot& Snapshot = State->GetSnapshot();
    if (Snapshot.RemainingSeconds > 0)
    {
        return;
    }

    if (Snapshot.Round < Snapshot.TotalRounds)
    {
        State->BeginRound(Snapshot.Round + 1, RoundSeconds);
    }
    else
    {
        const FName Winner = Snapshot.RedCorner.Health >= Snapshot.BlueCorner.Health
            ? Snapshot.RedCorner.FighterId
            : Snapshot.BlueCorner.FighterId;
        State->CompleteMatch(Winner);
    }
}

void ACombatLabGameMode::HandleSimulationTick()
{
    ACombatMatchGameState* State = GetGameState<ACombatMatchGameState>();
    if (!State || State->GetSnapshot().Phase != ECombatMatchPhase::InProgress)
    {
        return;
    }

    ++SimulationStep;
    State->RecoverStamina(2.75f);
    if (SimulationStep % 3 == 0)
    {
        const bool bRedAttacks = SimulationStep % 2 == 0;
        const float Damage = bRedAttacks ? 4.5f : 3.5f;
        State->ApplyStrike(bRedAttacks, Damage, 6.0f);
        EvaluateMatchEnd();
    }
}

void ACombatLabGameMode::EvaluateMatchEnd()
{
    ACombatMatchGameState* State = GetGameState<ACombatMatchGameState>();
    if (!State)
    {
        return;
    }

    const FCombatMatchSnapshot& Snapshot = State->GetSnapshot();
    if (Snapshot.RedCorner.Health <= 0.0f)
    {
        State->CompleteMatch(Snapshot.BlueCorner.FighterId);
    }
    else if (Snapshot.BlueCorner.Health <= 0.0f)
    {
        State->CompleteMatch(Snapshot.RedCorner.FighterId);
    }
}
