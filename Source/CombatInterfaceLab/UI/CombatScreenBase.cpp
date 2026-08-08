#include "UI/CombatScreenBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"

void UCombatScreenBase::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    if (!WidgetTree->RootWidget)
    {
        BuildScreen();
        FinalizeNavigation();
    }
}

void UCombatScreenBase::Activate()
{
    if (NavigationButtons.Num() > 0 && NavigationButtons[0] && GetOwningPlayer())
    {
        NavigationButtons[0]->SetUserFocus(GetOwningPlayer());
    }
}

UTextBlock* UCombatScreenBase::MakeText(const FText& Text, const int32 Size, const FLinearColor& Color)
{
    UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>();
    Block->SetText(Text);
    Block->SetColorAndOpacity(FSlateColor(Color));
    Block->SetAutoWrapText(true);
    FSlateFontInfo Font = Block->GetFont();
    Font.Size = Size;
    Block->SetFont(Font);
    return Block;
}

UButton* UCombatScreenBase::MakeButton(const FText& Text, UTextBlock*& OutLabel)
{
    UButton* Button = WidgetTree->ConstructWidget<UButton>();
    Button->SetBackgroundColor(FLinearColor(0.12f, 0.13f, 0.15f, 1.0f));
    OutLabel = MakeText(Text, 22);
    OutLabel->SetJustification(ETextJustify::Center);
    OutLabel->SetMargin(FMargin(18.0f, 12.0f));
    Button->AddChild(OutLabel);
    RegisterButton(Button);
    return Button;
}

USpacer* UCombatScreenBase::MakeSpacer(const float Height)
{
    USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>();
    Spacer->SetSize(FVector2D(1.0f, Height));
    return Spacer;
}

void UCombatScreenBase::RegisterButton(UButton* Button)
{
    if (Button)
    {
        NavigationButtons.Add(Button);
    }
}

void UCombatScreenBase::FinalizeNavigation()
{
    for (int32 Index = 0; Index < NavigationButtons.Num(); ++Index)
    {
        UButton* Current = NavigationButtons[Index];
        UButton* Previous = NavigationButtons[(Index - 1 + NavigationButtons.Num()) % NavigationButtons.Num()];
        UButton* Next = NavigationButtons[(Index + 1) % NavigationButtons.Num()];
        Current->SetNavigationRuleExplicit(EUINavigation::Up, Previous);
        Current->SetNavigationRuleExplicit(EUINavigation::Left, Previous);
        Current->SetNavigationRuleExplicit(EUINavigation::Down, Next);
        Current->SetNavigationRuleExplicit(EUINavigation::Right, Next);
    }
}
