#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "State/CombatLabTypes.h"
#include "CombatMatchGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FCombatSnapshotChanged, const FCombatMatchSnapshot&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCombatVitalsChanged, bool, const FCombatFighterSnapshot&);
DECLARE_MULTICAST_DELEGATE_OneParam(FCombatTimerChanged, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCombatRoundChanged, int32, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FCombatPhaseChanged, ECombatMatchPhase);

/** Replicated server-owned state. RepNotify delegates are the only path into the HUD. */
UCLASS()
class COMBATINTERFACELAB_API ACombatMatchGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ACombatMatchGameState();

    const FCombatMatchSnapshot& GetSnapshot() const { return MatchSnapshot; }

    FCombatSnapshotChanged OnSnapshotChanged;
    FCombatVitalsChanged OnVitalsChanged;
    FCombatTimerChanged OnTimerChanged;
    FCombatRoundChanged OnRoundChanged;
    FCombatPhaseChanged OnPhaseChanged;

    void InitializeMatch(FName RedFighter, FName BlueFighter, int32 RoundSeconds, int32 TotalRounds);
    void ApplyStrike(bool bRedAttacks, float Damage, float StaminaCost);
    void RecoverStamina(float Amount);
    void AdvanceClock();
    void BeginRound(int32 NewRound, int32 RoundSeconds);
    void CompleteMatch(FName WinnerId);
    void ApplyResynchronizedSnapshot(const FCombatMatchSnapshot& AuthoritativeSnapshot);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing=OnRep_MatchSnapshot)
    FCombatMatchSnapshot MatchSnapshot;

    UFUNCTION()
    void OnRep_MatchSnapshot(FCombatMatchSnapshot PreviousSnapshot);

    void Publish(const FCombatMatchSnapshot& PreviousSnapshot);
    void BroadcastDiff(const FCombatMatchSnapshot& PreviousSnapshot);
};
