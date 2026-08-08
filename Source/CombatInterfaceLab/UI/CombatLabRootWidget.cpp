#include "UI/CombatLabRootWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SafeZone.h"
#include "Components/ScaleBox.h"
#include "Components/ScaleBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "TimerManager.h"
#include "UI/CombatScreens.h"
#include "UI/CombatUIPresenter.h"

#define LOCTEXT_NAMESPACE "CombatLabRoot"

void UCombatLabRootWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildInterface();
}

void UCombatLabRootWidget::NativeDestruct()
{
    if (Presenter)
    {
        Presenter->OnScreenChanged.RemoveAll(this);
        Presenter->OnSnapshotChanged.RemoveAll(this);
        Presenter->OnAccessibilityChanged.RemoveAll(this);
        Presenter->OnConnectionChanged.RemoveAll(this);
        Presenter->OnFighterSelectionChanged.RemoveAll(this);
        Presenter->OnSubtitleRequested.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UCombatLabRootWidget::SetPresenter(UCombatUIPresenter* InPresenter)
{
    Presenter = InPresenter;
    if (!Presenter)
    {
        return;
    }

    Presenter->OnScreenChanged.AddUObject(this, &UCombatLabRootWidget::HandleScreenChanged);
    Presenter->OnSnapshotChanged.AddUObject(this, &UCombatLabRootWidget::HandleSnapshotChanged);
    Presenter->OnAccessibilityChanged.AddUObject(this, &UCombatLabRootWidget::HandleAccessibilityChanged);
    Presenter->OnConnectionChanged.AddUObject(this, &UCombatLabRootWidget::HandleConnectionChanged);
    Presenter->OnFighterSelectionChanged.AddUObject(this, &UCombatLabRootWidget::HandleSelectionChanged);
    Presenter->OnSubtitleRequested.AddUObject(this, &UCombatLabRootWidget::HandleSubtitle);

    FighterSelectScreen->SetPresenter(Presenter);
    SettingsScreen->SetPresenter(Presenter);
    MatchHudScreen->SetPresenter(Presenter);
    PauseScreen->SetPresenter(Presenter);
    ResultsScreen->SetPresenter(Presenter);

    HandleAccessibilityChanged(Presenter->GetAccessibilitySettings());
    HandleConnectionChanged(Presenter->GetConnectionStatus());
    HandleSnapshotChanged(Presenter->GetSnapshot());
    HandleScreenChanged(Presenter->GetCurrentScreen());
}

void UCombatLabRootWidget::BuildInterface()
{
    // Global invalidation is enabled in DefaultEngine.ini. A nested
    // UInvalidationBox here caused stale overlay layers when the HUD updated
    // more frequently than the connection banner.
    UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>();
    WidgetTree->RootWidget = Overlay;

    USafeZone* SafeZone = WidgetTree->ConstructWidget<USafeZone>();
    Overlay->AddChildToOverlay(SafeZone);

    UScaleBox* ResponsiveScale = WidgetTree->ConstructWidget<UScaleBox>();
    ResponsiveScale->SetStretch(EStretch::ScaleToFit);
    ResponsiveScale->SetStretchDirection(EStretchDirection::Both);
    SafeZone->AddChild(ResponsiveScale);

    USizeBox* DesignSurface = WidgetTree->ConstructWidget<USizeBox>();
    DesignSurface->SetWidthOverride(1920.0f);
    DesignSurface->SetHeightOverride(1080.0f);
    ResponsiveScale->AddChild(DesignSurface);
    if (UScaleBoxSlot* SurfaceSlot = Cast<UScaleBoxSlot>(DesignSurface->Slot))
    {
        SurfaceSlot->SetHorizontalAlignment(HAlign_Center);
        SurfaceSlot->SetVerticalAlignment(VAlign_Center);
    }

    InterfaceScaleBox = WidgetTree->ConstructWidget<UScaleBox>();
    InterfaceScaleBox->SetStretch(EStretch::UserSpecified);
    InterfaceScaleBox->SetUserSpecifiedScale(1.0f);
    DesignSurface->AddChild(InterfaceScaleBox);

    // UserSpecified ScaleBox children otherwise collapse to each screen's
    // desired width, producing a different layout footprint per screen.
    USizeBox* InterfaceSurface = WidgetTree->ConstructWidget<USizeBox>();
    InterfaceSurface->SetWidthOverride(1920.0f);
    InterfaceSurface->SetHeightOverride(1080.0f);
    InterfaceScaleBox->AddChild(InterfaceSurface);

    ScreenSwitcher = WidgetTree->ConstructWidget<UWidgetSwitcher>();
    InterfaceSurface->AddChild(ScreenSwitcher);
    FighterSelectScreen = WidgetTree->ConstructWidget<UCombatFighterSelectScreen>();
    SettingsScreen = WidgetTree->ConstructWidget<UCombatSettingsScreen>();
    MatchHudScreen = WidgetTree->ConstructWidget<UCombatMatchHudScreen>();
    PauseScreen = WidgetTree->ConstructWidget<UCombatPauseScreen>();
    ResultsScreen = WidgetTree->ConstructWidget<UCombatResultsScreen>();
    ScreenSwitcher->AddChild(FighterSelectScreen);
    ScreenSwitcher->AddChild(SettingsScreen);
    ScreenSwitcher->AddChild(MatchHudScreen);
    ScreenSwitcher->AddChild(PauseScreen);
    ScreenSwitcher->AddChild(ResultsScreen);

    ConnectionBanner = WidgetTree->ConstructWidget<UBorder>();
    ConnectionBanner->SetPadding(FMargin(14.0f, 8.0f));
    ConnectionText = WidgetTree->ConstructWidget<UTextBlock>();
    FSlateFontInfo StatusFont = ConnectionText->GetFont();
    StatusFont.Size = 15;
    ConnectionText->SetFont(StatusFont);
    ConnectionBanner->AddChild(ConnectionText);
    UOverlaySlot* ConnectionSlot = Overlay->AddChildToOverlay(ConnectionBanner);
    ConnectionSlot->SetHorizontalAlignment(HAlign_Right);
    ConnectionSlot->SetVerticalAlignment(VAlign_Top);
    ConnectionSlot->SetPadding(FMargin(20.0f));

    SubtitlePanel = WidgetTree->ConstructWidget<UBorder>();
    SubtitlePanel->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
    SubtitlePanel->SetPadding(FMargin(22.0f, 12.0f));
    SubtitlePanel->SetVisibility(ESlateVisibility::Collapsed);
    SubtitleText = WidgetTree->ConstructWidget<UTextBlock>();
    SubtitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    SubtitleText->SetJustification(ETextJustify::Center);
    FSlateFontInfo SubtitleFont = SubtitleText->GetFont();
    SubtitleFont.Size = 22;
    SubtitleText->SetFont(SubtitleFont);
    SubtitlePanel->AddChild(SubtitleText);
    UOverlaySlot* SubtitleSlot = Overlay->AddChildToOverlay(SubtitlePanel);
    SubtitleSlot->SetHorizontalAlignment(HAlign_Center);
    SubtitleSlot->SetVerticalAlignment(VAlign_Bottom);
    SubtitleSlot->SetPadding(FMargin(80.0f, 80.0f));
}

void UCombatLabRootWidget::HandleScreenChanged(const ECombatLabScreen Screen)
{
    const int32 Index = static_cast<int32>(Screen);
    ScreenSwitcher->SetActiveWidgetIndex(Index);
    switch (Screen)
    {
    case ECombatLabScreen::FighterSelect: FighterSelectScreen->Activate(); break;
    case ECombatLabScreen::Settings: SettingsScreen->Refresh(); SettingsScreen->Activate(); break;
    case ECombatLabScreen::MatchHud: MatchHudScreen->Activate(); break;
    case ECombatLabScreen::Pause: PauseScreen->Activate(); break;
    case ECombatLabScreen::Results: ResultsScreen->UpdateSnapshot(Presenter->GetSnapshot()); ResultsScreen->Activate(); break;
    }

    // Screen replacement can introduce a new cached paint layer. Refresh the
    // persistent status lane so it remains above that layer without polling.
    if (Presenter)
    {
        HandleConnectionChanged(Presenter->GetConnectionStatus());
    }
}

void UCombatLabRootWidget::HandleSnapshotChanged(const FCombatMatchSnapshot& Snapshot)
{
    MatchHudScreen->UpdateSnapshot(Snapshot);
    ResultsScreen->UpdateSnapshot(Snapshot);
}

void UCombatLabRootWidget::HandleAccessibilityChanged(const FCombatAccessibilitySettings& Settings)
{
    InterfaceScaleBox->SetUserSpecifiedScale(Settings.InterfaceScale);
    MatchHudScreen->ApplyPalette(Presenter->GetPalette());
    SettingsScreen->Refresh();
    if (!Settings.bSubtitlesEnabled)
    {
        HideSubtitle();
    }
}

void UCombatLabRootWidget::HandleConnectionChanged(const ECombatConnectionStatus Status)
{
    FText StatusText;
    FLinearColor Color;
    switch (Status)
    {
    case ECombatConnectionStatus::Interrupted:
        StatusText = LOCTEXT("ConnectionInterrupted", "●  CONNECTION INTERRUPTED");
        Color = FLinearColor(0.88f, 0.08f, 0.06f, 0.96f);
        break;
    case ECombatConnectionStatus::Reconnecting:
        StatusText = LOCTEXT("ConnectionReconnecting", "↻  RECONNECTING");
        Color = FLinearColor(0.9f, 0.5f, 0.02f, 0.96f);
        break;
    case ECombatConnectionStatus::Synchronizing:
        StatusText = LOCTEXT("ConnectionSynchronizing", "↻  SYNCHRONIZING AUTHORITATIVE STATE");
        Color = FLinearColor(0.9f, 0.5f, 0.02f, 0.96f);
        break;
    default:
        StatusText = LOCTEXT("ConnectionOnline", "●  CONNECTED / EVENT STREAM LIVE");
        Color = FLinearColor(0.03f, 0.42f, 0.2f, 0.94f);
        break;
    }
    ConnectionText->SetText(StatusText);
    ConnectionBanner->SetBrushColor(Color);
}

void UCombatLabRootWidget::HandleSelectionChanged(const int32 Index)
{
    FighterSelectScreen->Activate();
}

void UCombatLabRootWidget::HandleSubtitle(const FText& Text)
{
    SubtitleText->SetText(Text);
    SubtitlePanel->SetVisibility(ESlateVisibility::HitTestInvisible);
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SubtitleTimer);
        GetWorld()->GetTimerManager().SetTimer(SubtitleTimer, this, &UCombatLabRootWidget::HideSubtitle, 2.8f, false);
    }
}

void UCombatLabRootWidget::HideSubtitle()
{
    if (SubtitlePanel)
    {
        SubtitlePanel->SetVisibility(ESlateVisibility::Collapsed);
    }
}

#undef LOCTEXT_NAMESPACE
