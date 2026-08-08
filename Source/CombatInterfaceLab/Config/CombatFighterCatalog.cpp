#include "Config/CombatFighterCatalog.h"

#include "CombatInterfaceLab.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "CombatFighters"

namespace
{
FString ParseLuaValue(const FString& Line)
{
    FString Left;
    FString Right;
    if (!Line.Split(TEXT("="), &Left, &Right))
    {
        return FString();
    }

    Right.TrimStartAndEndInline();
    Right.RemoveFromEnd(TEXT(","));
    Right.TrimStartAndEndInline();
    if (Right.Len() >= 2 && Right.StartsWith(TEXT("\"") ) && Right.EndsWith(TEXT("\"")))
    {
        Right = Right.Mid(1, Right.Len() - 2);
    }
    return Right;
}

FString ParseLuaKey(const FString& Line)
{
    FString Left;
    FString Right;
    if (Line.Split(TEXT("="), &Left, &Right))
    {
        Left.TrimStartAndEndInline();
        return Left;
    }
    return FString();
}

FCombatFighterDefinition BuildDefinition(const TMap<FString, FString>& Fields)
{
    FCombatFighterDefinition Definition;
    Definition.Id = FName(Fields.FindRef(TEXT("id")));
    Definition.NameKey = FName(Fields.FindRef(TEXT("name_key")));
    Definition.DisplayName = FCombatFighterCatalog::ResolveDisplayName(
        Definition.NameKey,
        Fields.FindRef(TEXT("fallback_name")));
    Definition.Nickname = FText::FromString(Fields.FindRef(TEXT("nickname")));
    Definition.Country = FText::FromString(Fields.FindRef(TEXT("country")));
    Definition.Archetype = FText::FromString(Fields.FindRef(TEXT("archetype")));
    Definition.Power = FCString::Atoi(*Fields.FindRef(TEXT("power")));
    Definition.Speed = FCString::Atoi(*Fields.FindRef(TEXT("speed")));
    Definition.Grappling = FCString::Atoi(*Fields.FindRef(TEXT("grappling")));
    return Definition;
}
}

TArray<FCombatFighterDefinition> FCombatFighterCatalog::Load()
{
    FString Source;
    const FString Path = FPaths::ProjectContentDir() / TEXT("Config/Fighters.lua");
    if (!FFileHelper::LoadFileToString(Source, *Path))
    {
        UE_LOG(LogCombatInterfaceLab, Warning, TEXT("Could not read %s; using built-in fighter data."), *Path);
        return Defaults();
    }

    TArray<FCombatFighterDefinition> Result;
    TArray<FString> Lines;
    Source.ParseIntoArrayLines(Lines);
    TMap<FString, FString> Fields;
    bool bInsideFighter = false;

    for (FString Line : Lines)
    {
        Line.TrimStartAndEndInline();
        if (Line.IsEmpty() || Line.StartsWith(TEXT("--")))
        {
            continue;
        }
        if (Line.StartsWith(TEXT("fighter")) && Line.Contains(TEXT("{")))
        {
            Fields.Reset();
            bInsideFighter = true;
            continue;
        }
        if (bInsideFighter && Line.StartsWith(TEXT("}")))
        {
            FCombatFighterDefinition Definition = BuildDefinition(Fields);
            if (!Definition.Id.IsNone() && !Definition.DisplayName.IsEmpty())
            {
                Result.Add(MoveTemp(Definition));
            }
            bInsideFighter = false;
            continue;
        }
        if (bInsideFighter && Line.Contains(TEXT("=")))
        {
            Fields.Add(ParseLuaKey(Line), ParseLuaValue(Line));
        }
    }

    if (Result.Num() < 2)
    {
        UE_LOG(LogCombatInterfaceLab, Warning, TEXT("Fighters.lua produced fewer than two valid fighters; using defaults."));
        return Defaults();
    }

    UE_LOG(LogCombatInterfaceLab, Display, TEXT("Loaded %d fighter profiles from Lua data."), Result.Num());
    return Result;
}

FText FCombatFighterCatalog::ResolveDisplayName(const FName NameKey, const FString& Fallback)
{
    if (NameKey == TEXT("Fighter.MarisolVega"))
    {
        return LOCTEXT("Fighter.MarisolVega", "MARISOL VEGA");
    }
    if (NameKey == TEXT("Fighter.AmaraOkafor"))
    {
        return LOCTEXT("Fighter.AmaraOkafor", "AMARA OKAFOR");
    }
    if (NameKey == TEXT("Fighter.SofiaKovac"))
    {
        return LOCTEXT("Fighter.SofiaKovac", "SOFIA KOVAC");
    }
    if (NameKey == TEXT("Fighter.KenjiSato"))
    {
        return LOCTEXT("Fighter.KenjiSato", "KENJI SATO");
    }
    return FText::FromString(Fallback);
}

TArray<FCombatFighterDefinition> FCombatFighterCatalog::Defaults()
{
    TArray<FCombatFighterDefinition> Result;
    const struct FDefaultRow
    {
        const TCHAR* Id;
        const TCHAR* Key;
        const TCHAR* Name;
        const TCHAR* Nickname;
        const TCHAR* Country;
        const TCHAR* Archetype;
        int32 Power;
        int32 Speed;
        int32 Grappling;
    } Rows[] = {
        { TEXT("marisol_vega"), TEXT("Fighter.MarisolVega"), TEXT("MARISOL VEGA"), TEXT("THE TEMPEST"), TEXT("MEXICO"), TEXT("PRESSURE BOXER"), 88, 82, 67 },
        { TEXT("amara_okafor"), TEXT("Fighter.AmaraOkafor"), TEXT("AMARA OKAFOR"), TEXT("NIGHTFALL"), TEXT("NIGERIA"), TEXT("COUNTER STRIKER"), 79, 91, 72 },
        { TEXT("sofia_kovac"), TEXT("Fighter.SofiaKovac"), TEXT("SOFIA KOVAC"), TEXT("THE ANVIL"), TEXT("CROATIA"), TEXT("WRESTLER"), 84, 70, 94 },
        { TEXT("kenji_sato"), TEXT("Fighter.KenjiSato"), TEXT("KENJI SATO"), TEXT("ZERO HOUR"), TEXT("JAPAN"), TEXT("KICKBOXER"), 81, 89, 75 }
    };

    for (const FDefaultRow& Row : Rows)
    {
        FCombatFighterDefinition Definition;
        Definition.Id = FName(Row.Id);
        Definition.NameKey = FName(Row.Key);
        Definition.DisplayName = ResolveDisplayName(Definition.NameKey, Row.Name);
        Definition.Nickname = FText::FromString(Row.Nickname);
        Definition.Country = FText::FromString(Row.Country);
        Definition.Archetype = FText::FromString(Row.Archetype);
        Definition.Power = Row.Power;
        Definition.Speed = Row.Speed;
        Definition.Grappling = Row.Grappling;
        Result.Add(MoveTemp(Definition));
    }
    return Result;
}

#undef LOCTEXT_NAMESPACE
