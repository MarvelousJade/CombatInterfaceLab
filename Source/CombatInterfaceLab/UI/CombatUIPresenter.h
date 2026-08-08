#pragma once

#include "CoreMinimal.h"
#include "State/CombatLabTypes.h"
#include "UObject/Object.h"
#include "CombatUIPresenter.generated.h"

class ACombatLabPlayerController;
class ACombatMatchGameState;

DECLARE_MULTICAST_DELEGATE_OneParam(FCombatScreenChanged, ECombatLabScreen);
DECLARE_MULTICAST_DELEGATE_OneParam(FCombatViewSnapshotChanged, const FCombatMatchSnapshot&);
DECLARE_MULTICAST_DELEGATE_OneParam(FCombatAccessibilityChanged, const FCombatAccessibilitySettings&);
DECLARE_MULTICAST_DELEGATE_OneParam(FCombatConnectionChanged, ECombatConnectionStatus);
DECLARE_MULTICAST_DELEGATE_OneParam(FCombatFighterSelectionChanged, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FCombatSubtitleRequested, const FText&);

/** C++ presenter: owns flow/accessibility decisions and exposes immutable events to UMG. */
UCLASS()
class COMBATINTERFACELAB_API UCombatUIPresenter : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(ACombatLabPlayerController* InOwner);
    virtual void BeginDestroy() override;

    FCombatScreenChanged OnScreenChanged;
    FCombatViewSnapshotChanged OnSnapshotChanged;
    FCombatAccessibilityChanged OnAccessibilityChanged;
    FCombatConnectionChanged OnConnectionChanged;
    FCombatFighterSelectionChanged OnFighterSelectionChanged;
    FCombatSubtitleRequested OnSubtitleRequested;

    ECombatLabScreen GetCurrentScreen() const { return CurrentScreen; }
    ECombatConnectionStatus GetConnectionStatus() const { return ConnectionStatus; }
    const FCombatAccessibilitySettings& GetAccessibilitySettings() const { return Accessibility; }
    const TArray<FCombatFighterDefinition>& GetFighters() const { return Fighters; }
    int32 GetSelectedFighterIndex() const { return SelectedFighterIndex; }
    const FCombatMatchSnapshot& GetSnapshot() const;
    FCombatPalette GetPalette() const;
    FText FighterName(FName FighterId) const;

    void SelectFighter(int32 Index);
    void StartMatch();
    void OpenSettings();
    void CloseSettings();
    void PauseMatch();
    void ResumeMatch();
    void ReturnToSelection();
    void Rematch();
    void Strike();
    void RequestConnectionSimulation();

    void CycleColorVisionMode();
    void ToggleSubtitles();
    void ToggleReducedMotion();
    void CycleInterfaceScale();

    void SetConnectionStatus(ECombatConnectionStatus NewStatus);
    void ApplyResynchronizedSnapshot(const FCombatMatchSnapshot& Snapshot);
    void ShowScreenForAutomation(ECombatLabScreen Screen);

private:
    UPROPERTY()
    TObjectPtr<ACombatLabPlayerController> Owner;

    UPROPERTY()
    TObjectPtr<ACombatMatchGameState> MatchState;

    TArray<FCombatFighterDefinition> Fighters;
    FCombatAccessibilitySettings Accessibility;
    ECombatLabScreen CurrentScreen = ECombatLabScreen::FighterSelect;
    ECombatLabScreen SettingsReturnScreen = ECombatLabScreen::FighterSelect;
    ECombatConnectionStatus ConnectionStatus = ECombatConnectionStatus::Connected;
    int32 SelectedFighterIndex = 0;
    FCombatMatchSnapshot EmptySnapshot;

    void TransitionTo(ECombatLabScreen NewScreen, bool bForce = false);
    void HandleSnapshot(const FCombatMatchSnapshot& Snapshot);
    void Announce(const FText& Text);
};
