#include "State/CombatMatchGameState.h"

#include "Net/UnrealNetwork.h"
#include "State/CombatMatchModel.h"

ACombatMatchGameState::ACombatMatchGameState()
{
    bReplicates = true;
    SetNetUpdateFrequency(10.0f);
}

void ACombatMatchGameState::InitializeMatch(
    const FName RedFighter,
    const FName BlueFighter,
    const int32 RoundSeconds,
    const int32 TotalRounds)
{
    if (!HasAuthority())
    {
        return;
    }

    const FCombatMatchSnapshot Previous = MatchSnapshot;
    MatchSnapshot = FCombatMatchSnapshot();
    MatchSnapshot.RedCorner.FighterId = RedFighter;
    MatchSnapshot.BlueCorner.FighterId = BlueFighter;
    MatchSnapshot.Round = 1;
    MatchSnapshot.TotalRounds = FMath::Max(1, TotalRounds);
    MatchSnapshot.RemainingSeconds = FMath::Max(1, RoundSeconds);
    MatchSnapshot.Phase = ECombatMatchPhase::InProgress;
    MatchSnapshot.Revision = Previous.Revision;
    Publish(Previous);
}

void ACombatMatchGameState::ApplyStrike(
    const bool bRedAttacks,
    const float Damage,
    const float StaminaCost)
{
    if (!HasAuthority() || MatchSnapshot.Phase != ECombatMatchPhase::InProgress)
    {
        return;
    }

    const FCombatMatchSnapshot Previous = MatchSnapshot;
    FCombatFighterSnapshot& Attacker = bRedAttacks ? MatchSnapshot.RedCorner : MatchSnapshot.BlueCorner;
    FCombatFighterSnapshot& Defender = bRedAttacks ? MatchSnapshot.BlueCorner : MatchSnapshot.RedCorner;
    FCombatMatchModel::ApplyStrike(Attacker, Defender, Damage, StaminaCost);
    Publish(Previous);
}

void ACombatMatchGameState::RecoverStamina(const float Amount)
{
    if (!HasAuthority() || MatchSnapshot.Phase != ECombatMatchPhase::InProgress)
    {
        return;
    }

    const FCombatMatchSnapshot Previous = MatchSnapshot;
    MatchSnapshot.RedCorner.Stamina = FMath::Min(
        MatchSnapshot.RedCorner.MaxStamina,
        MatchSnapshot.RedCorner.Stamina + FMath::Max(0.0f, Amount));
    MatchSnapshot.BlueCorner.Stamina = FMath::Min(
        MatchSnapshot.BlueCorner.MaxStamina,
        MatchSnapshot.BlueCorner.Stamina + FMath::Max(0.0f, Amount));
    Publish(Previous);
}

void ACombatMatchGameState::AdvanceClock()
{
    if (!HasAuthority() || MatchSnapshot.Phase != ECombatMatchPhase::InProgress)
    {
        return;
    }

    const FCombatMatchSnapshot Previous = MatchSnapshot;
    MatchSnapshot.RemainingSeconds = FMath::Max(0, MatchSnapshot.RemainingSeconds - 1);
    Publish(Previous);
}

void ACombatMatchGameState::BeginRound(const int32 NewRound, const int32 RoundSeconds)
{
    if (!HasAuthority())
    {
        return;
    }

    const FCombatMatchSnapshot Previous = MatchSnapshot;
    MatchSnapshot.Round = FMath::Clamp(NewRound, 1, MatchSnapshot.TotalRounds);
    MatchSnapshot.RemainingSeconds = FMath::Max(1, RoundSeconds);
    MatchSnapshot.RedCorner.Health = MatchSnapshot.RedCorner.MaxHealth;
    MatchSnapshot.BlueCorner.Health = MatchSnapshot.BlueCorner.MaxHealth;
    MatchSnapshot.RedCorner.Stamina = MatchSnapshot.RedCorner.MaxStamina;
    MatchSnapshot.BlueCorner.Stamina = MatchSnapshot.BlueCorner.MaxStamina;
    MatchSnapshot.Phase = ECombatMatchPhase::InProgress;
    Publish(Previous);
}

void ACombatMatchGameState::CompleteMatch(const FName WinnerId)
{
    if (!HasAuthority())
    {
        return;
    }

    const FCombatMatchSnapshot Previous = MatchSnapshot;
    MatchSnapshot.WinnerId = WinnerId;
    MatchSnapshot.Phase = ECombatMatchPhase::Complete;
    Publish(Previous);
}

void ACombatMatchGameState::ApplyResynchronizedSnapshot(const FCombatMatchSnapshot& AuthoritativeSnapshot)
{
    const FCombatMatchSnapshot Previous = MatchSnapshot;
    MatchSnapshot = FCombatMatchModel::Reconcile(MatchSnapshot, AuthoritativeSnapshot);
    if (MatchSnapshot.Revision != Previous.Revision)
    {
        BroadcastDiff(Previous);
    }
}

void ACombatMatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACombatMatchGameState, MatchSnapshot);
}

void ACombatMatchGameState::OnRep_MatchSnapshot(FCombatMatchSnapshot PreviousSnapshot)
{
    BroadcastDiff(PreviousSnapshot);
}

void ACombatMatchGameState::Publish(const FCombatMatchSnapshot& PreviousSnapshot)
{
    ++MatchSnapshot.Revision;
    ForceNetUpdate();
    BroadcastDiff(PreviousSnapshot);
}

void ACombatMatchGameState::BroadcastDiff(const FCombatMatchSnapshot& PreviousSnapshot)
{
    const auto FighterChanged = [](const FCombatFighterSnapshot& A, const FCombatFighterSnapshot& B)
    {
        return A.FighterId != B.FighterId
            || !FMath::IsNearlyEqual(A.Health, B.Health)
            || !FMath::IsNearlyEqual(A.Stamina, B.Stamina)
            || A.SignificantStrikes != B.SignificantStrikes;
    };

    if (FighterChanged(PreviousSnapshot.RedCorner, MatchSnapshot.RedCorner))
    {
        OnVitalsChanged.Broadcast(true, MatchSnapshot.RedCorner);
    }
    if (FighterChanged(PreviousSnapshot.BlueCorner, MatchSnapshot.BlueCorner))
    {
        OnVitalsChanged.Broadcast(false, MatchSnapshot.BlueCorner);
    }
    if (PreviousSnapshot.RemainingSeconds != MatchSnapshot.RemainingSeconds)
    {
        OnTimerChanged.Broadcast(MatchSnapshot.RemainingSeconds);
    }
    if (PreviousSnapshot.Round != MatchSnapshot.Round || PreviousSnapshot.TotalRounds != MatchSnapshot.TotalRounds)
    {
        OnRoundChanged.Broadcast(MatchSnapshot.Round, MatchSnapshot.TotalRounds);
    }
    if (PreviousSnapshot.Phase != MatchSnapshot.Phase)
    {
        OnPhaseChanged.Broadcast(MatchSnapshot.Phase);
    }
    OnSnapshotChanged.Broadcast(MatchSnapshot);
}
