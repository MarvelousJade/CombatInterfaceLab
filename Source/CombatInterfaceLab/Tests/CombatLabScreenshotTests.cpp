#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "AutomationBlueprintFunctionLibrary.h"
#include "Misc/AutomationTest.h"
#include "Player/CombatLabPlayerController.h"
#include "Tests/AutomationCommon.h"
#include "UI/CombatUIPresenter.h"

namespace
{
ACombatLabPlayerController* FindLabController()
{
    if (UWorld* World = AutomationCommon::GetAnyGameWorld())
    {
        return Cast<ACombatLabPlayerController>(World->GetFirstPlayerController());
    }
    return nullptr;
}
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCombatSetScreenCommand, ECombatLabScreen, Screen);
bool FCombatSetScreenCommand::Update()
{
    if (ACombatLabPlayerController* Controller = FindLabController())
    {
        if (UCombatUIPresenter* Presenter = Controller->GetPresenter())
        {
            if (Screen == ECombatLabScreen::MatchHud)
            {
                Presenter->StartMatch();
            }
            else if (Screen == ECombatLabScreen::Results)
            {
                Controller->CombatLabFinishMatch();
            }
            Presenter->ShowScreenForAutomation(Screen);
        }
    }
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
    FCombatCaptureUIScreenCommand,
    FString,
    ScreenshotName,
    FAutomationTestBase*,
    Test);
bool FCombatCaptureUIScreenCommand::Update()
{
    UWorld* World = AutomationCommon::GetAnyGameWorld();
    if (!World)
    {
        Test->AddError(TEXT("No game world was available for the UI screenshot."));
        return true;
    }

    FAutomationScreenshotOptions Options =
        UAutomationBlueprintFunctionLibrary::GetDefaultScreenshotOptionsForRendering(EComparisonTolerance::Low, 0.0f);
    // Keep every screen at the approved reference resolution. This test is
    // intended for -game/-RenderOffscreen; editor PIE windows add title-bar
    // geometry that cannot be resized reliably between captures.
    Options.Resolution = FVector2D(1600.0f, 900.0f);
    Options.FrameDelay = 1;
    // The framework permits only one in-flight comparison. Keep this latent
    // command queued until the previous screenshot has completed.
    return UAutomationBlueprintFunctionLibrary::TakeAutomationScreenshotOfUI_Immediate(
        World,
        ScreenshotName,
        Options);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCombatUIScreenComparisonTest,
    "CombatInterfaceLab.Screenshots.AllScreens",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FCombatUIScreenComparisonTest::RunTest(const FString& Parameters)
{
    AutomationOpenMap(TEXT("/Engine/Maps/Entry"));
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));

    const struct FScreenShotCase
    {
        ECombatLabScreen Screen;
        const TCHAR* Name;
    } Cases[] = {
        { ECombatLabScreen::FighterSelect, TEXT("FighterSelect") },
        { ECombatLabScreen::Settings, TEXT("Settings") },
        { ECombatLabScreen::MatchHud, TEXT("MatchHUD") },
        { ECombatLabScreen::Pause, TEXT("Pause") },
        { ECombatLabScreen::Results, TEXT("Results") }
    };

    for (const FScreenShotCase& Case : Cases)
    {
        ADD_LATENT_AUTOMATION_COMMAND(FCombatSetScreenCommand(Case.Screen));
        ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.25f));
        ADD_LATENT_AUTOMATION_COMMAND(FCombatCaptureUIScreenCommand(Case.Name, this));
        ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.25f));
    }
    return true;
}

#endif
