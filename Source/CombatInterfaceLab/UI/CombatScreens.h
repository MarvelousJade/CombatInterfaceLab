#pragma once

#include "CoreMinimal.h"
#include "State/CombatLabTypes.h"
#include "UI/CombatScreenBase.h"
#include "CombatScreens.generated.h"

class UBorder;
class UButton;
class UCombatUIPresenter;
class UProgressBar;
class UTextBlock;

UCLASS()
class COMBATINTERFACELAB_API UCombatFighterSelectScreen : public UCombatScreenBase
{
    GENERATED_BODY()

public:
    void SetPresenter(UCombatUIPresenter* InPresenter);
    virtual void Activate() override;

protected:
    virtual void BuildScreen() override;

private:
    UPROPERTY()
    TObjectPtr<UCombatUIPresenter> Presenter;
    UPROPERTY()
    TArray<TObjectPtr<UTextBlock>> FighterLabels;
    UPROPERTY()
    TObjectPtr<UTextBlock> FighterName;
    UPROPERTY()
    TObjectPtr<UTextBlock> FighterBio;
    UPROPERTY()
    TObjectPtr<UTextBlock> FighterPower;
    UPROPERTY()
    TObjectPtr<UTextBlock> FighterSpeed;
    UPROPERTY()
    TObjectPtr<UTextBlock> FighterGrappling;

    UFUNCTION() void ChooseFighter0();
    UFUNCTION() void ChooseFighter1();
    UFUNCTION() void ChooseFighter2();
    UFUNCTION() void ChooseFighter3();
    UFUNCTION() void ConfirmFighter();
    UFUNCTION() void OpenSettings();
    void RefreshSelection();
};

UCLASS()
class COMBATINTERFACELAB_API UCombatSettingsScreen : public UCombatScreenBase
{
    GENERATED_BODY()

public:
    void SetPresenter(UCombatUIPresenter* InPresenter);
    void Refresh();

protected:
    virtual void BuildScreen() override;

private:
    UPROPERTY() TObjectPtr<UCombatUIPresenter> Presenter;
    UPROPERTY() TObjectPtr<UTextBlock> ColorModeLabel;
    UPROPERTY() TObjectPtr<UTextBlock> SubtitleLabel;
    UPROPERTY() TObjectPtr<UTextBlock> MotionLabel;
    UPROPERTY() TObjectPtr<UTextBlock> ScaleLabel;

    UFUNCTION() void CycleColorMode();
    UFUNCTION() void ToggleSubtitles();
    UFUNCTION() void ToggleMotion();
    UFUNCTION() void CycleScale();
    UFUNCTION() void GoBack();
};

UCLASS()
class COMBATINTERFACELAB_API UCombatMatchHudScreen : public UCombatScreenBase
{
    GENERATED_BODY()

public:
    void SetPresenter(UCombatUIPresenter* InPresenter);
    void UpdateSnapshot(const FCombatMatchSnapshot& Snapshot);
    void ApplyPalette(const FCombatPalette& Palette);

protected:
    virtual void BuildScreen() override;

private:
    UPROPERTY() TObjectPtr<UCombatUIPresenter> Presenter;
    UPROPERTY() TObjectPtr<UTextBlock> RedName;
    UPROPERTY() TObjectPtr<UTextBlock> BlueName;
    UPROPERTY() TObjectPtr<UTextBlock> RedValue;
    UPROPERTY() TObjectPtr<UTextBlock> BlueValue;
    UPROPERTY() TObjectPtr<UTextBlock> TimerValue;
    UPROPERTY() TObjectPtr<UTextBlock> RoundValue;
    UPROPERTY() TObjectPtr<UProgressBar> RedHealth;
    UPROPERTY() TObjectPtr<UProgressBar> RedStamina;
    UPROPERTY() TObjectPtr<UProgressBar> BlueHealth;
    UPROPERTY() TObjectPtr<UProgressBar> BlueStamina;

    UFUNCTION() void Strike();
    UFUNCTION() void Pause();
    UFUNCTION() void SimulateNetworkIssue();
};

UCLASS()
class COMBATINTERFACELAB_API UCombatPauseScreen : public UCombatScreenBase
{
    GENERATED_BODY()

public:
    void SetPresenter(UCombatUIPresenter* InPresenter);

protected:
    virtual void BuildScreen() override;

private:
    UPROPERTY() TObjectPtr<UCombatUIPresenter> Presenter;
    UFUNCTION() void Resume();
    UFUNCTION() void OpenSettings();
    UFUNCTION() void Reconnect();
    UFUNCTION() void QuitToSelection();
};

UCLASS()
class COMBATINTERFACELAB_API UCombatResultsScreen : public UCombatScreenBase
{
    GENERATED_BODY()

public:
    void SetPresenter(UCombatUIPresenter* InPresenter);
    void UpdateSnapshot(const FCombatMatchSnapshot& Snapshot);

protected:
    virtual void BuildScreen() override;

private:
    UPROPERTY() TObjectPtr<UCombatUIPresenter> Presenter;
    UPROPERTY() TObjectPtr<UTextBlock> WinnerText;
    UPROPERTY() TObjectPtr<UTextBlock> ResultStats;
    UFUNCTION() void Rematch();
    UFUNCTION() void ReturnToSelection();
};
