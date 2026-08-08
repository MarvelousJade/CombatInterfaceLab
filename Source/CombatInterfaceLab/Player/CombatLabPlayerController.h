#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "State/CombatLabTypes.h"
#include "CombatLabPlayerController.generated.h"

class UCombatLabRootWidget;
class UCombatUIPresenter;

UCLASS()
class COMBATINTERFACELAB_API ACombatLabPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UCombatUIPresenter* GetPresenter() const { return Presenter; }

    void RequestStartMatch(FName FighterId);
    void RequestPlayerStrike();
    void SetDemoPaused(bool bPaused);
    void SimulateConnectionInterruption();

    UFUNCTION(Exec)
    void CombatLabFinishMatch();

protected:
    UFUNCTION(Server, Reliable)
    void ServerStartMatch(FName FighterId);

    UFUNCTION(Server, Reliable)
    void ServerPlayerStrike();

    UFUNCTION(Server, Reliable)
    void ServerRequestStateResync(int32 ClientRevision);

    UFUNCTION(Server, Reliable)
    void ServerFinishMatch();

    UFUNCTION(Client, Reliable)
    void ClientReceiveStateResync(const FCombatMatchSnapshot& Snapshot);

private:
    UPROPERTY()
    TObjectPtr<UCombatUIPresenter> Presenter;

    UPROPERTY()
    TObjectPtr<UCombatLabRootWidget> RootWidget;

    FTimerHandle ReconnectTimer;
    FTimerHandle ResyncTimer;

    void HandleCancelInput();
    void HandleConnectionShortcut();
    void BeginReconnect();
    void BeginResynchronize();
};
