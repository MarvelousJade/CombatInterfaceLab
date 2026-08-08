#pragma once

#include "CoreMinimal.h"
#include "CombatLabTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatLabScreen : uint8
{
    FighterSelect,
    Settings,
    MatchHud,
    Pause,
    Results
};

UENUM(BlueprintType)
enum class ECombatMatchPhase : uint8
{
    PreMatch,
    InProgress,
    RoundBreak,
    Complete
};

UENUM(BlueprintType)
enum class ECombatConnectionStatus : uint8
{
    Connected,
    Interrupted,
    Reconnecting,
    Synchronizing
};

UENUM(BlueprintType)
enum class ECombatColorVisionMode : uint8
{
    Standard,
    Deuteranopia,
    Protanopia,
    Tritanopia
};

USTRUCT(BlueprintType)
struct FCombatAccessibilitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSubtitlesEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bReducedMotion = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombatColorVisionMode ColorVisionMode = ECombatColorVisionMode::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InterfaceScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FCombatPalette
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FLinearColor RedCorner = FLinearColor(0.93f, 0.035f, 0.07f, 1.0f);

    UPROPERTY(BlueprintReadOnly)
    FLinearColor BlueCorner = FLinearColor(0.0f, 0.55f, 1.0f, 1.0f);

    UPROPERTY(BlueprintReadOnly)
    FLinearColor Accent = FLinearColor(1.0f, 0.78f, 0.04f, 1.0f);

    UPROPERTY(BlueprintReadOnly)
    FLinearColor Critical = FLinearColor(1.0f, 0.22f, 0.08f, 1.0f);
};

USTRUCT(BlueprintType)
struct FCombatFighterDefinition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FName Id = NAME_None;

    UPROPERTY(BlueprintReadOnly)
    FName NameKey = NAME_None;

    UPROPERTY(BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(BlueprintReadOnly)
    FText Nickname;

    UPROPERTY(BlueprintReadOnly)
    FText Country;

    UPROPERTY(BlueprintReadOnly)
    FText Archetype;

    UPROPERTY(BlueprintReadOnly)
    int32 Power = 75;

    UPROPERTY(BlueprintReadOnly)
    int32 Speed = 75;

    UPROPERTY(BlueprintReadOnly)
    int32 Grappling = 75;
};

USTRUCT(BlueprintType)
struct FCombatFighterSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FName FighterId = NAME_None;

    UPROPERTY(BlueprintReadOnly)
    float Health = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    float MaxStamina = 100.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 SignificantStrikes = 0;

    float GetHealthFraction() const
    {
        return MaxHealth > 0.0f ? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f) : 0.0f;
    }

    float GetStaminaFraction() const
    {
        return MaxStamina > 0.0f ? FMath::Clamp(Stamina / MaxStamina, 0.0f, 1.0f) : 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FCombatMatchSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FCombatFighterSnapshot RedCorner;

    UPROPERTY(BlueprintReadOnly)
    FCombatFighterSnapshot BlueCorner;

    UPROPERTY(BlueprintReadOnly)
    int32 Round = 1;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalRounds = 3;

    UPROPERTY(BlueprintReadOnly)
    int32 RemainingSeconds = 60;

    UPROPERTY(BlueprintReadOnly)
    ECombatMatchPhase Phase = ECombatMatchPhase::PreMatch;

    UPROPERTY(BlueprintReadOnly)
    FName WinnerId = NAME_None;

    UPROPERTY(BlueprintReadOnly)
    int32 Revision = 0;
};
