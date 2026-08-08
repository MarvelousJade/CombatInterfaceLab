#include "UI/CombatScreens.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/CombatUIPresenter.h"

#define LOCTEXT_NAMESPACE "CombatScreens"

namespace
{
void StyleRoot(UBorder* Root)
{
    Root->SetBrushColor(FLinearColor(0.018f, 0.021f, 0.027f, 1.0f));
    // Reserve the top-right status lane so the persistent connection banner
    // never obscures fighter names or screen headings.
    Root->SetPadding(FMargin(54.0f, 72.0f, 54.0f, 38.0f));
}

void SetFill(UVerticalBoxSlot* Slot)
{
    FSlateChildSize Size;
    Size.SizeRule = ESlateSizeRule::Fill;
    Slot->SetSize(Size);
}

void SetFill(UHorizontalBoxSlot* Slot)
{
    FSlateChildSize Size;
    Size.SizeRule = ESlateSizeRule::Fill;
    Slot->SetSize(Size);
}

FText OnOff(const bool bEnabled)
{
    return bEnabled ? LOCTEXT("On", "ON") : LOCTEXT("Off", "OFF");
}
}

void UCombatFighterSelectScreen::BuildScreen()
{
    UBorder* Root = WidgetTree->ConstructWidget<UBorder>();
    StyleRoot(Root);
    WidgetTree->RootWidget = Root;

    UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
    Root->AddChild(Layout);

    UTextBlock* Kicker = MakeText(LOCTEXT("SelectKicker", "COMBAT INTERFACE LAB  /  FIGHT CARD 01"), 18, FLinearColor(1.0f, 0.78f, 0.04f));
    Layout->AddChildToVerticalBox(Kicker);
    UTextBlock* Title = MakeText(LOCTEXT("SelectTitle", "CHOOSE YOUR CORNER"), 54);
    Layout->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 20.0f));

    UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>();
    SetFill(Layout->AddChildToVerticalBox(Body));

    UVerticalBox* Roster = WidgetTree->ConstructWidget<UVerticalBox>();
    UHorizontalBoxSlot* RosterSlot = Body->AddChildToHorizontalBox(Roster);
    SetFill(RosterSlot);
    RosterSlot->SetPadding(FMargin(0.0f, 0.0f, 22.0f, 0.0f));

    const FText Placeholders[] = {
        LOCTEXT("FighterOne", "FIGHTER 01"),
        LOCTEXT("FighterTwo", "FIGHTER 02"),
        LOCTEXT("FighterThree", "FIGHTER 03"),
        LOCTEXT("FighterFour", "FIGHTER 04")
    };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        UTextBlock* Label = nullptr;
        UButton* Button = MakeButton(Placeholders[Index], Label);
        FighterLabels.Add(Label);
        Roster->AddChildToVerticalBox(Button)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
        switch (Index)
        {
        case 0: Button->OnClicked.AddDynamic(this, &UCombatFighterSelectScreen::ChooseFighter0); break;
        case 1: Button->OnClicked.AddDynamic(this, &UCombatFighterSelectScreen::ChooseFighter1); break;
        case 2: Button->OnClicked.AddDynamic(this, &UCombatFighterSelectScreen::ChooseFighter2); break;
        default: Button->OnClicked.AddDynamic(this, &UCombatFighterSelectScreen::ChooseFighter3); break;
        }
    }

    UBorder* Card = WidgetTree->ConstructWidget<UBorder>();
    Card->SetBrushColor(FLinearColor(0.055f, 0.06f, 0.07f, 1.0f));
    Card->SetPadding(FMargin(42.0f));
    UHorizontalBoxSlot* CardSlot = Body->AddChildToHorizontalBox(Card);
    SetFill(CardSlot);

    UVerticalBox* CardLayout = WidgetTree->ConstructWidget<UVerticalBox>();
    Card->AddChild(CardLayout);
    CardLayout->AddChildToVerticalBox(MakeText(LOCTEXT("TaleTape", "TALE OF THE TAPE"), 17, FLinearColor(0.65f, 0.67f, 0.7f)));
    FighterName = MakeText(LOCTEXT("SelectPrompt", "SELECT A FIGHTER"), 44);
    CardLayout->AddChildToVerticalBox(FighterName)->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 8.0f));
    FighterBio = MakeText(FText::GetEmpty(), 22, FLinearColor(1.0f, 0.78f, 0.04f));
    CardLayout->AddChildToVerticalBox(FighterBio);
    CardLayout->AddChildToVerticalBox(MakeSpacer(34.0f));
    FighterStats = MakeText(FText::GetEmpty(), 27);
    CardLayout->AddChildToVerticalBox(FighterStats);
    CardLayout->AddChildToVerticalBox(MakeSpacer(32.0f));
    UTextBlock* Hint = MakeText(LOCTEXT("InputHint", "D-PAD / ARROWS  Navigate     A / ENTER  Confirm     Mouse supported"), 17, FLinearColor(0.67f, 0.69f, 0.72f));
    CardLayout->AddChildToVerticalBox(Hint);

    UHorizontalBox* Actions = WidgetTree->ConstructWidget<UHorizontalBox>();
    Layout->AddChildToVerticalBox(Actions)->SetPadding(FMargin(0.0f, 22.0f, 0.0f, 0.0f));
    UTextBlock* ConfirmLabel = nullptr;
    UButton* Confirm = MakeButton(LOCTEXT("StartMatch", "START MATCH"), ConfirmLabel);
    Confirm->OnClicked.AddDynamic(this, &UCombatFighterSelectScreen::ConfirmFighter);
    SetFill(Actions->AddChildToHorizontalBox(Confirm));
    UTextBlock* SettingsLabel = nullptr;
    UButton* Settings = MakeButton(LOCTEXT("Settings", "ACCESSIBILITY & SETTINGS"), SettingsLabel);
    Settings->OnClicked.AddDynamic(this, &UCombatFighterSelectScreen::OpenSettings);
    UHorizontalBoxSlot* SettingsSlot = Actions->AddChildToHorizontalBox(Settings);
    SetFill(SettingsSlot);
    SettingsSlot->SetPadding(FMargin(14.0f, 0.0f, 0.0f, 0.0f));
}

void UCombatFighterSelectScreen::SetPresenter(UCombatUIPresenter* InPresenter)
{
    Presenter = InPresenter;
    RefreshSelection();
}

void UCombatFighterSelectScreen::Activate()
{
    RefreshSelection();
    Super::Activate();
}

void UCombatFighterSelectScreen::ChooseFighter0() { if (Presenter) Presenter->SelectFighter(0); RefreshSelection(); }
void UCombatFighterSelectScreen::ChooseFighter1() { if (Presenter) Presenter->SelectFighter(1); RefreshSelection(); }
void UCombatFighterSelectScreen::ChooseFighter2() { if (Presenter) Presenter->SelectFighter(2); RefreshSelection(); }
void UCombatFighterSelectScreen::ChooseFighter3() { if (Presenter) Presenter->SelectFighter(3); RefreshSelection(); }
void UCombatFighterSelectScreen::ConfirmFighter() { if (Presenter) Presenter->StartMatch(); }
void UCombatFighterSelectScreen::OpenSettings() { if (Presenter) Presenter->OpenSettings(); }

void UCombatFighterSelectScreen::RefreshSelection()
{
    if (!Presenter || !FighterName || !FighterStats)
    {
        return;
    }

    const TArray<FCombatFighterDefinition>& Fighters = Presenter->GetFighters();
    for (int32 Index = 0; Index < FighterLabels.Num(); ++Index)
    {
        if (Fighters.IsValidIndex(Index))
        {
            FighterLabels[Index]->SetText(Fighters[Index].DisplayName);
            FighterLabels[Index]->SetColorAndOpacity(FSlateColor(
                Index == Presenter->GetSelectedFighterIndex()
                    ? Presenter->GetPalette().Accent
                    : FLinearColor::White));
        }
    }

    if (!Fighters.IsValidIndex(Presenter->GetSelectedFighterIndex()))
    {
        return;
    }
    const FCombatFighterDefinition& Fighter = Fighters[Presenter->GetSelectedFighterIndex()];
    FighterName->SetText(Fighter.DisplayName);
    FighterBio->SetText(FText::Format(
        LOCTEXT("FighterBioFormat", "{0}  •  {1}  •  {2}"),
        Fighter.Nickname,
        Fighter.Country,
        Fighter.Archetype));
    FighterStats->SetText(FText::Format(
        LOCTEXT("StatsFormat", "POWER             {0}\nSPEED             {1}\nGRAPPLING         {2}"),
        FText::AsNumber(Fighter.Power),
        FText::AsNumber(Fighter.Speed),
        FText::AsNumber(Fighter.Grappling)));
}

void UCombatSettingsScreen::BuildScreen()
{
    UBorder* Root = WidgetTree->ConstructWidget<UBorder>();
    StyleRoot(Root);
    WidgetTree->RootWidget = Root;
    UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
    Root->AddChild(Layout);

    Layout->AddChildToVerticalBox(MakeText(LOCTEXT("SettingsKicker", "ACCESSIBILITY PROFILE"), 18, FLinearColor(1.0f, 0.78f, 0.04f)));
    Layout->AddChildToVerticalBox(MakeText(LOCTEXT("SettingsTitle", "SETTINGS"), 54))->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 24.0f));

    UTextBlock* Label = nullptr;
    UTextBlock* SettingLabel = nullptr;
    UButton* Color = MakeButton(FText::GetEmpty(), SettingLabel);
    ColorModeLabel = SettingLabel;
    Color->OnClicked.AddDynamic(this, &UCombatSettingsScreen::CycleColorMode);
    Layout->AddChildToVerticalBox(Color)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    UButton* Subtitles = MakeButton(FText::GetEmpty(), SettingLabel);
    SubtitleLabel = SettingLabel;
    Subtitles->OnClicked.AddDynamic(this, &UCombatSettingsScreen::ToggleSubtitles);
    Layout->AddChildToVerticalBox(Subtitles)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    UButton* Motion = MakeButton(FText::GetEmpty(), SettingLabel);
    MotionLabel = SettingLabel;
    Motion->OnClicked.AddDynamic(this, &UCombatSettingsScreen::ToggleMotion);
    Layout->AddChildToVerticalBox(Motion)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    UButton* Scale = MakeButton(FText::GetEmpty(), SettingLabel);
    ScaleLabel = SettingLabel;
    Scale->OnClicked.AddDynamic(this, &UCombatSettingsScreen::CycleScale);
    Layout->AddChildToVerticalBox(Scale)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    Layout->AddChildToVerticalBox(MakeText(
        LOCTEXT("AccessibilityNote", "Colour is never the only status cue. Reduced motion keeps transitions immediate and suppresses non-essential pulses. Subtitles include speaker labels."),
        18,
        FLinearColor(0.68f, 0.7f, 0.73f)))->SetPadding(FMargin(0.0f, 18.0f));
    UButton* Back = MakeButton(LOCTEXT("Back", "BACK"), Label);
    Back->OnClicked.AddDynamic(this, &UCombatSettingsScreen::GoBack);
    Layout->AddChildToVerticalBox(Back);
}

void UCombatSettingsScreen::SetPresenter(UCombatUIPresenter* InPresenter)
{
    Presenter = InPresenter;
    Refresh();
}

void UCombatSettingsScreen::Refresh()
{
    if (!Presenter || !ColorModeLabel)
    {
        return;
    }
    const FCombatAccessibilitySettings& Settings = Presenter->GetAccessibilitySettings();
    FText ColorName;
    switch (Settings.ColorVisionMode)
    {
    case ECombatColorVisionMode::Deuteranopia: ColorName = LOCTEXT("Deuteranopia", "DEUTERANOPIA"); break;
    case ECombatColorVisionMode::Protanopia: ColorName = LOCTEXT("Protanopia", "PROTANOPIA"); break;
    case ECombatColorVisionMode::Tritanopia: ColorName = LOCTEXT("Tritanopia", "TRITANOPIA"); break;
    default: ColorName = LOCTEXT("Standard", "STANDARD"); break;
    }
    ColorModeLabel->SetText(FText::Format(LOCTEXT("ColorModeFormat", "COLOUR-BLIND PALETTE     {0}"), ColorName));
    SubtitleLabel->SetText(FText::Format(LOCTEXT("SubtitlesFormat", "SUBTITLES     {0}"), OnOff(Settings.bSubtitlesEnabled)));
    MotionLabel->SetText(FText::Format(LOCTEXT("MotionFormat", "REDUCED MOTION     {0}"), OnOff(Settings.bReducedMotion)));
    ScaleLabel->SetText(FText::Format(
        LOCTEXT("ScaleFormat", "INTERFACE SCALE     {0}%"),
        FText::AsNumber(FMath::RoundToInt(Settings.InterfaceScale * 100.0f))));
}

void UCombatSettingsScreen::CycleColorMode() { if (Presenter) Presenter->CycleColorVisionMode(); Refresh(); }
void UCombatSettingsScreen::ToggleSubtitles() { if (Presenter) Presenter->ToggleSubtitles(); Refresh(); }
void UCombatSettingsScreen::ToggleMotion() { if (Presenter) Presenter->ToggleReducedMotion(); Refresh(); }
void UCombatSettingsScreen::CycleScale() { if (Presenter) Presenter->CycleInterfaceScale(); Refresh(); }
void UCombatSettingsScreen::GoBack() { if (Presenter) Presenter->CloseSettings(); }

void UCombatMatchHudScreen::BuildScreen()
{
    UBorder* Root = WidgetTree->ConstructWidget<UBorder>();
    StyleRoot(Root);
    WidgetTree->RootWidget = Root;
    UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
    Root->AddChild(Layout);

    UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>();
    SetFill(Layout->AddChildToVerticalBox(Header));

    UVerticalBox* RedColumn = WidgetTree->ConstructWidget<UVerticalBox>();
    SetFill(Header->AddChildToHorizontalBox(RedColumn));
    RedName = MakeText(LOCTEXT("RedCorner", "RED CORNER"), 30);
    RedColumn->AddChildToVerticalBox(RedName);
    RedHealth = WidgetTree->ConstructWidget<UProgressBar>();
    RedColumn->AddChildToVerticalBox(RedHealth)->SetPadding(FMargin(0.0f, 10.0f, 20.0f, 8.0f));
    RedStamina = WidgetTree->ConstructWidget<UProgressBar>();
    RedColumn->AddChildToVerticalBox(RedStamina)->SetPadding(FMargin(0.0f, 0.0f, 20.0f, 8.0f));
    RedValue = MakeText(FText::GetEmpty(), 20);
    RedColumn->AddChildToVerticalBox(RedValue);

    UVerticalBox* Center = WidgetTree->ConstructWidget<UVerticalBox>();
    UHorizontalBoxSlot* CenterSlot = Header->AddChildToHorizontalBox(Center);
    CenterSlot->SetHorizontalAlignment(HAlign_Center);
    CenterSlot->SetPadding(FMargin(26.0f));
    RoundValue = MakeText(LOCTEXT("Round", "ROUND 1 / 3"), 19, FLinearColor(1.0f, 0.78f, 0.04f));
    RoundValue->SetJustification(ETextJustify::Center);
    Center->AddChildToVerticalBox(RoundValue);
    TimerValue = MakeText(LOCTEXT("Timer", "01:00"), 58);
    TimerValue->SetJustification(ETextJustify::Center);
    Center->AddChildToVerticalBox(TimerValue);
    UTextBlock* Authority = MakeText(LOCTEXT("Authority", "SERVER AUTHORITATIVE"), 14, FLinearColor(0.6f, 0.62f, 0.65f));
    Authority->SetJustification(ETextJustify::Center);
    Center->AddChildToVerticalBox(Authority);

    UVerticalBox* BlueColumn = WidgetTree->ConstructWidget<UVerticalBox>();
    SetFill(Header->AddChildToHorizontalBox(BlueColumn));
    BlueName = MakeText(LOCTEXT("BlueCorner", "BLUE CORNER"), 30);
    BlueName->SetJustification(ETextJustify::Right);
    BlueColumn->AddChildToVerticalBox(BlueName);
    BlueHealth = WidgetTree->ConstructWidget<UProgressBar>();
    BlueColumn->AddChildToVerticalBox(BlueHealth)->SetPadding(FMargin(20.0f, 10.0f, 0.0f, 8.0f));
    BlueStamina = WidgetTree->ConstructWidget<UProgressBar>();
    BlueColumn->AddChildToVerticalBox(BlueStamina)->SetPadding(FMargin(20.0f, 0.0f, 0.0f, 8.0f));
    BlueValue = MakeText(FText::GetEmpty(), 20);
    BlueValue->SetJustification(ETextJustify::Right);
    BlueColumn->AddChildToVerticalBox(BlueValue);

    UBorder* Octagon = WidgetTree->ConstructWidget<UBorder>();
    Octagon->SetBrushColor(FLinearColor(0.035f, 0.039f, 0.046f, 1.0f));
    Octagon->SetPadding(FMargin(36.0f));
    UTextBlock* OctagonText = MakeText(
        LOCTEXT("OctagonText", "LIVE MATCH TELEMETRY\n\nUI updates only when replicated state delegates fire.\nNo widget in this screen polls on Tick."),
        25,
        FLinearColor(0.74f, 0.76f, 0.79f));
    OctagonText->SetJustification(ETextJustify::Center);
    Octagon->AddChild(OctagonText);
    Layout->AddChildToVerticalBox(Octagon)->SetPadding(FMargin(0.0f, 20.0f));

    UHorizontalBox* Actions = WidgetTree->ConstructWidget<UHorizontalBox>();
    Layout->AddChildToVerticalBox(Actions);
    UTextBlock* Label = nullptr;
    UButton* StrikeButton = MakeButton(LOCTEXT("Strike", "THROW STRIKE"), Label);
    StrikeButton->OnClicked.AddDynamic(this, &UCombatMatchHudScreen::Strike);
    SetFill(Actions->AddChildToHorizontalBox(StrikeButton));
    UButton* NetworkButton = MakeButton(LOCTEXT("Interrupt", "SIMULATE INTERRUPTION  [F8]"), Label);
    NetworkButton->OnClicked.AddDynamic(this, &UCombatMatchHudScreen::SimulateNetworkIssue);
    UHorizontalBoxSlot* NetworkSlot = Actions->AddChildToHorizontalBox(NetworkButton);
    SetFill(NetworkSlot);
    NetworkSlot->SetPadding(FMargin(12.0f, 0.0f));
    UButton* PauseButton = MakeButton(LOCTEXT("Pause", "PAUSE  [ESC / MENU]"), Label);
    PauseButton->OnClicked.AddDynamic(this, &UCombatMatchHudScreen::Pause);
    SetFill(Actions->AddChildToHorizontalBox(PauseButton));
}

void UCombatMatchHudScreen::SetPresenter(UCombatUIPresenter* InPresenter)
{
    Presenter = InPresenter;
    if (Presenter)
    {
        UpdateSnapshot(Presenter->GetSnapshot());
        ApplyPalette(Presenter->GetPalette());
    }
}

void UCombatMatchHudScreen::UpdateSnapshot(const FCombatMatchSnapshot& Snapshot)
{
    if (!RedHealth || !Presenter)
    {
        return;
    }
    RedName->SetText(Presenter->FighterName(Snapshot.RedCorner.FighterId));
    BlueName->SetText(Presenter->FighterName(Snapshot.BlueCorner.FighterId));
    RedHealth->SetPercent(Snapshot.RedCorner.GetHealthFraction());
    RedStamina->SetPercent(Snapshot.RedCorner.GetStaminaFraction());
    BlueHealth->SetPercent(Snapshot.BlueCorner.GetHealthFraction());
    BlueStamina->SetPercent(Snapshot.BlueCorner.GetStaminaFraction());
    RedValue->SetText(FText::Format(
        LOCTEXT("CornerValue", "HP {0}  •  ST {1}  •  SIG {2}"),
        FText::AsNumber(FMath::RoundToInt(Snapshot.RedCorner.Health)),
        FText::AsNumber(FMath::RoundToInt(Snapshot.RedCorner.Stamina)),
        FText::AsNumber(Snapshot.RedCorner.SignificantStrikes)));
    BlueValue->SetText(FText::Format(
        LOCTEXT("CornerValueBlue", "SIG {2}  •  ST {1}  •  HP {0}"),
        FText::AsNumber(FMath::RoundToInt(Snapshot.BlueCorner.Health)),
        FText::AsNumber(FMath::RoundToInt(Snapshot.BlueCorner.Stamina)),
        FText::AsNumber(Snapshot.BlueCorner.SignificantStrikes)));
    TimerValue->SetText(FText::Format(
        LOCTEXT("TimerFormat", "{0}:{1}"),
        FText::AsNumber(Snapshot.RemainingSeconds / 60),
        FText::FromString(FString::Printf(TEXT("%02d"), Snapshot.RemainingSeconds % 60))));
    RoundValue->SetText(FText::Format(
        LOCTEXT("RoundFormat", "ROUND {0} / {1}"),
        FText::AsNumber(Snapshot.Round),
        FText::AsNumber(Snapshot.TotalRounds)));
}

void UCombatMatchHudScreen::ApplyPalette(const FCombatPalette& Palette)
{
    if (!RedHealth)
    {
        return;
    }
    RedHealth->SetFillColorAndOpacity(Palette.RedCorner);
    BlueHealth->SetFillColorAndOpacity(Palette.BlueCorner);
    RedStamina->SetFillColorAndOpacity(Palette.Accent);
    BlueStamina->SetFillColorAndOpacity(Palette.Accent);
    RedName->SetColorAndOpacity(FSlateColor(Palette.RedCorner));
    BlueName->SetColorAndOpacity(FSlateColor(Palette.BlueCorner));
}

void UCombatMatchHudScreen::Strike() { if (Presenter) Presenter->Strike(); }
void UCombatMatchHudScreen::Pause() { if (Presenter) Presenter->PauseMatch(); }
void UCombatMatchHudScreen::SimulateNetworkIssue() { if (Presenter) Presenter->RequestConnectionSimulation(); }

void UCombatPauseScreen::BuildScreen()
{
    UBorder* Root = WidgetTree->ConstructWidget<UBorder>();
    StyleRoot(Root);
    WidgetTree->RootWidget = Root;
    UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
    Root->AddChild(Layout);
    Layout->AddChildToVerticalBox(MakeText(LOCTEXT("PauseKicker", "MATCH CONTROL"), 18, FLinearColor(1.0f, 0.78f, 0.04f)));
    Layout->AddChildToVerticalBox(MakeText(LOCTEXT("Paused", "PAUSED"), 60))->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 25.0f));
    UTextBlock* Label = nullptr;
    UButton* ResumeButton = MakeButton(LOCTEXT("Resume", "RESUME MATCH"), Label);
    ResumeButton->OnClicked.AddDynamic(this, &UCombatPauseScreen::Resume);
    Layout->AddChildToVerticalBox(ResumeButton)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    UButton* SettingsButton = MakeButton(LOCTEXT("PauseSettings", "ACCESSIBILITY & SETTINGS"), Label);
    SettingsButton->OnClicked.AddDynamic(this, &UCombatPauseScreen::OpenSettings);
    Layout->AddChildToVerticalBox(SettingsButton)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    UButton* ReconnectButton = MakeButton(LOCTEXT("Reconnect", "TEST RECONNECTION / RESYNC"), Label);
    ReconnectButton->OnClicked.AddDynamic(this, &UCombatPauseScreen::Reconnect);
    Layout->AddChildToVerticalBox(ReconnectButton)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    UButton* QuitButton = MakeButton(LOCTEXT("QuitSelection", "RETURN TO FIGHTER SELECT"), Label);
    QuitButton->OnClicked.AddDynamic(this, &UCombatPauseScreen::QuitToSelection);
    Layout->AddChildToVerticalBox(QuitButton);
}

void UCombatPauseScreen::SetPresenter(UCombatUIPresenter* InPresenter) { Presenter = InPresenter; }
void UCombatPauseScreen::Resume() { if (Presenter) Presenter->ResumeMatch(); }
void UCombatPauseScreen::OpenSettings() { if (Presenter) Presenter->OpenSettings(); }
void UCombatPauseScreen::Reconnect() { if (Presenter) Presenter->RequestConnectionSimulation(); }
void UCombatPauseScreen::QuitToSelection() { if (Presenter) Presenter->ReturnToSelection(); }

void UCombatResultsScreen::BuildScreen()
{
    UBorder* Root = WidgetTree->ConstructWidget<UBorder>();
    StyleRoot(Root);
    WidgetTree->RootWidget = Root;
    UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
    Root->AddChild(Layout);
    Layout->AddChildToVerticalBox(MakeText(LOCTEXT("OfficialResult", "OFFICIAL RESULT"), 18, FLinearColor(1.0f, 0.78f, 0.04f)));
    WinnerText = MakeText(LOCTEXT("WinnerPlaceholder", "WINNER"), 55);
    Layout->AddChildToVerticalBox(WinnerText)->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 18.0f));
    ResultStats = MakeText(FText::GetEmpty(), 27);
    Layout->AddChildToVerticalBox(ResultStats)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 28.0f));
    UTextBlock* Label = nullptr;
    UButton* RematchButton = MakeButton(LOCTEXT("Rematch", "REMATCH"), Label);
    RematchButton->OnClicked.AddDynamic(this, &UCombatResultsScreen::Rematch);
    Layout->AddChildToVerticalBox(RematchButton)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
    UButton* SelectionButton = MakeButton(LOCTEXT("ResultsSelection", "FIGHTER SELECT"), Label);
    SelectionButton->OnClicked.AddDynamic(this, &UCombatResultsScreen::ReturnToSelection);
    Layout->AddChildToVerticalBox(SelectionButton);
}

void UCombatResultsScreen::SetPresenter(UCombatUIPresenter* InPresenter)
{
    Presenter = InPresenter;
    if (Presenter)
    {
        UpdateSnapshot(Presenter->GetSnapshot());
    }
}

void UCombatResultsScreen::UpdateSnapshot(const FCombatMatchSnapshot& Snapshot)
{
    if (!WinnerText || !Presenter)
    {
        return;
    }
    WinnerText->SetText(FText::Format(LOCTEXT("WinnerFormat", "{0} WINS"), Presenter->FighterName(Snapshot.WinnerId)));
    ResultStats->SetText(FText::Format(
        LOCTEXT("ResultFormat", "ROUND                     {0}\nRED SIGNIFICANT STRIKES   {1}\nBLUE SIGNIFICANT STRIKES  {2}\nAUTHORITATIVE REVISION    {3}"),
        FText::AsNumber(Snapshot.Round),
        FText::AsNumber(Snapshot.RedCorner.SignificantStrikes),
        FText::AsNumber(Snapshot.BlueCorner.SignificantStrikes),
        FText::AsNumber(Snapshot.Revision)));
}

void UCombatResultsScreen::Rematch() { if (Presenter) Presenter->Rematch(); }
void UCombatResultsScreen::ReturnToSelection() { if (Presenter) Presenter->ReturnToSelection(); }

#undef LOCTEXT_NAMESPACE
