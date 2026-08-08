#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "State/CombatLabTypes.h"
#include "CombatLabRootWidget.generated.h"

class UBorder;
class UCombatFighterSelectScreen;
class UCombatMatchHudScreen;
class UCombatPauseScreen;
class UCombatResultsScreen;
class UCombatSettingsScreen;
class UCombatUIPresenter;
class UScaleBox;
class UTextBlock;
class UWidgetSwitcher;

/** Responsive UMG composition root. A single invalidated tree switches screens. */
UCLASS()
class COMBATINTERFACELAB_API UCombatLabRootWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetPresenter(UCombatUIPresenter* InPresenter);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

private:
    UPROPERTY() TObjectPtr<UCombatUIPresenter> Presenter;
    UPROPERTY() TObjectPtr<UWidgetSwitcher> ScreenSwitcher;
    UPROPERTY() TObjectPtr<UScaleBox> InterfaceScaleBox;
    UPROPERTY() TObjectPtr<UBorder> ConnectionBanner;
    UPROPERTY() TObjectPtr<UTextBlock> ConnectionText;
    UPROPERTY() TObjectPtr<UBorder> SubtitlePanel;
    UPROPERTY() TObjectPtr<UTextBlock> SubtitleText;
    UPROPERTY() TObjectPtr<UCombatFighterSelectScreen> FighterSelectScreen;
    UPROPERTY() TObjectPtr<UCombatSettingsScreen> SettingsScreen;
    UPROPERTY() TObjectPtr<UCombatMatchHudScreen> MatchHudScreen;
    UPROPERTY() TObjectPtr<UCombatPauseScreen> PauseScreen;
    UPROPERTY() TObjectPtr<UCombatResultsScreen> ResultsScreen;

    FTimerHandle SubtitleTimer;

    void BuildInterface();
    void HandleScreenChanged(ECombatLabScreen Screen);
    void HandleSnapshotChanged(const FCombatMatchSnapshot& Snapshot);
    void HandleAccessibilityChanged(const FCombatAccessibilitySettings& Settings);
    void HandleConnectionChanged(ECombatConnectionStatus Status);
    void HandleSelectionChanged(int32 Index);
    void HandleSubtitle(const FText& Text);

    UFUNCTION()
    void HideSubtitle();
};
