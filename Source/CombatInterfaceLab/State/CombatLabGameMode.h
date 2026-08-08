#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CombatLabGameMode.generated.h"

UCLASS()
class COMBATINTERFACELAB_API ACombatLabGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACombatLabGameMode();

    virtual void BeginPlay() override;

    void RequestStartDemoMatch(FName SelectedFighter);
    void HandlePlayerStrike();
    void FinishDemoMatch();

private:
    UPROPERTY(EditDefaultsOnly, Category="Demo")
    int32 RoundSeconds = 60;

    UPROPERTY(EditDefaultsOnly, Category="Demo")
    int32 TotalRounds = 3;

    FTimerHandle ClockTimer;
    FTimerHandle SimulationTimer;
    int32 SimulationStep = 0;

    void HandleClockTick();
    void HandleSimulationTick();
    void EvaluateMatchEnd();
};
