#include "Player/CombatLabPlayerController.h"

#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "State/CombatLabGameMode.h"
#include "State/CombatMatchGameState.h"
#include "TimerManager.h"
#include "UI/CombatLabRootWidget.h"
#include "UI/CombatUIPresenter.h"

void ACombatLabPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController())
    {
        return;
    }

    bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);

    Presenter = NewObject<UCombatUIPresenter>(this);
    Presenter->Initialize(this);

    RootWidget = CreateWidget<UCombatLabRootWidget>(this, UCombatLabRootWidget::StaticClass());
    if (RootWidget)
    {
        RootWidget->SetPresenter(Presenter);
        RootWidget->AddToViewport(100);
    }
}

void ACombatLabPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ACombatLabPlayerController::HandleCancelInput);
    InputComponent->BindKey(EKeys::Gamepad_Special_Right, IE_Pressed, this, &ACombatLabPlayerController::HandleCancelInput);
    InputComponent->BindKey(EKeys::F8, IE_Pressed, this, &ACombatLabPlayerController::HandleConnectionShortcut);
    InputComponent->BindKey(EKeys::F10, IE_Pressed, this, &ACombatLabPlayerController::CombatLabFinishMatch);
}

void ACombatLabPlayerController::RequestStartMatch(const FName FighterId)
{
    ServerStartMatch(FighterId);
}

void ACombatLabPlayerController::RequestPlayerStrike()
{
    ServerPlayerStrike();
}

void ACombatLabPlayerController::SetDemoPaused(const bool bPaused)
{
    if (GetNetMode() == NM_Standalone)
    {
        SetPause(bPaused);
    }
}

void ACombatLabPlayerController::SimulateConnectionInterruption()
{
    if (!Presenter || Presenter->GetConnectionStatus() != ECombatConnectionStatus::Connected)
    {
        return;
    }

    Presenter->SetConnectionStatus(ECombatConnectionStatus::Interrupted);
    GetWorldTimerManager().SetTimer(ReconnectTimer, this, &ACombatLabPlayerController::BeginReconnect, 0.7f, false);
}

void ACombatLabPlayerController::CombatLabFinishMatch()
{
    ServerFinishMatch();
}

void ACombatLabPlayerController::ServerStartMatch_Implementation(const FName FighterId)
{
    if (ACombatLabGameMode* GameMode = GetWorld()->GetAuthGameMode<ACombatLabGameMode>())
    {
        GameMode->RequestStartDemoMatch(FighterId);
    }
}

void ACombatLabPlayerController::ServerPlayerStrike_Implementation()
{
    if (ACombatLabGameMode* GameMode = GetWorld()->GetAuthGameMode<ACombatLabGameMode>())
    {
        GameMode->HandlePlayerStrike();
    }
}

void ACombatLabPlayerController::ServerRequestStateResync_Implementation(const int32 ClientRevision)
{
    if (const ACombatMatchGameState* State = GetWorld()->GetGameState<ACombatMatchGameState>())
    {
        UE_LOG(LogTemp, Display, TEXT("State resync request: client revision %d, server revision %d"), ClientRevision, State->GetSnapshot().Revision);
        ClientReceiveStateResync(State->GetSnapshot());
    }
}

void ACombatLabPlayerController::ServerFinishMatch_Implementation()
{
    if (ACombatLabGameMode* GameMode = GetWorld()->GetAuthGameMode<ACombatLabGameMode>())
    {
        GameMode->FinishDemoMatch();
    }
}

void ACombatLabPlayerController::ClientReceiveStateResync_Implementation(const FCombatMatchSnapshot& Snapshot)
{
    if (Presenter)
    {
        Presenter->ApplyResynchronizedSnapshot(Snapshot);
    }
}

void ACombatLabPlayerController::HandleCancelInput()
{
    if (!Presenter)
    {
        return;
    }

    switch (Presenter->GetCurrentScreen())
    {
    case ECombatLabScreen::MatchHud:
        Presenter->PauseMatch();
        break;
    case ECombatLabScreen::Pause:
        Presenter->ResumeMatch();
        break;
    case ECombatLabScreen::Settings:
        Presenter->CloseSettings();
        break;
    default:
        break;
    }
}

void ACombatLabPlayerController::HandleConnectionShortcut()
{
    SimulateConnectionInterruption();
}

void ACombatLabPlayerController::BeginReconnect()
{
    if (!Presenter)
    {
        return;
    }
    Presenter->SetConnectionStatus(ECombatConnectionStatus::Reconnecting);
    GetWorldTimerManager().SetTimer(ResyncTimer, this, &ACombatLabPlayerController::BeginResynchronize, 0.9f, false);
}

void ACombatLabPlayerController::BeginResynchronize()
{
    if (!Presenter)
    {
        return;
    }
    Presenter->SetConnectionStatus(ECombatConnectionStatus::Synchronizing);
    ServerRequestStateResync(Presenter->GetSnapshot().Revision);
}
