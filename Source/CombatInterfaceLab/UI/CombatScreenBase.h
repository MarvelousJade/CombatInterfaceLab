#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatScreenBase.generated.h"

class UButton;
class UPanelWidget;
class USpacer;
class UTextBlock;

UCLASS(Abstract)
class COMBATINTERFACELAB_API UCombatScreenBase : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void Activate();

protected:
    virtual void NativeOnInitialized() override;
    virtual void BuildScreen() PURE_VIRTUAL(UCombatScreenBase::BuildScreen, );

    UTextBlock* MakeText(const FText& Text, int32 Size = 24, const FLinearColor& Color = FLinearColor::White);
    UButton* MakeButton(const FText& Text, UTextBlock*& OutLabel);
    USpacer* MakeSpacer(float Height);
    void RegisterButton(UButton* Button);
    void FinalizeNavigation();

    UPROPERTY(Transient)
    TArray<TObjectPtr<UButton>> NavigationButtons;
};
