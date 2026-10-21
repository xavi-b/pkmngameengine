#include "movedef.h"

#include <algorithm>
#include <cctype>
#include <iostream>

namespace
{
char const* functionCodeNames[MoveDef::FunctionCodeCount] = {
    "None",
    "AddGhostTypeToTarget",
    "AddGrassTypeToTarget",
    "AddMoneyGainedFromBattle",
    "AddSpikesToFoeSide",
    "AddStealthRocksToFoeSide",
    "AddStickyWebToFoeSide",
    "AddToxicSpikesToFoeSide",
    "AllBattlersConsumeBerry",
    "AlwaysCriticalHit",
    "AttackAndSkipNextTurn",
    "AttackTwoTurnsLater",
    "AttackerFaintsIfUserFaints",
    "AttractTarget",
    "BadPoisonTarget",
    "BindTarget",
    "BindTargetDoublePowerIfTargetUnderwater",
    "BounceBackProblemCausingStatusMoves",
    "BurnAttackerBeforeUserActs",
    "BurnFlinchTarget",
    "BurnTarget",
    "BurnTargetIfTargetStatsRaisedThisTurn",
    "CannotBeRedirected",
    "CannotMakeTargetFaint",
    "CategoryDependsOnHigherDamageIgnoreTargetAbility",
    "CategoryDependsOnHigherDamagePoisonTarget",
    "ConfuseTarget",
    "ConfuseTargetAlwaysHitsInRainHitsTargetInSky",
    "CorrodeTargetItem",
    "CounterDamagePlusHalf",
    "CounterPhysicalDamage",
    "CounterSpecialDamage",
    "CrashDamageIfFailsUnusableInGravity",
    "CureTargetBurn",
    "CureTargetStatusHealUserHalfOfTotalHP",
    "CureUserBurnPoisonParalysis",
    "CureUserPartyStatus",
    "CurseTargetOrLowerUserSpd1RaiseUserAtkDef1",
    "DamageTargetAlly",
    "DestroyTargetBerryOrGem",
    "DisableTargetHealingMoves",
    "DisableTargetLastMoveUsed",
    "DisableTargetMovesKnownByUser",
    "DisableTargetSoundMoves",
    "DisableTargetStatusMoves",
    "DisableTargetUsingDifferentMove",
    "DisableTargetUsingSameMoveConsecutively",
    "DoesNothingCongratulations",
    "DoesNothingFailsIfNoAlly",
    "DoesNothingUnusableInGravity",
    "DoubleMoneyGainedFromBattle",
    "DoublePowerAfterFusionBolt",
    "DoublePowerAfterFusionFlare",
    "DoublePowerIfAllyFaintedLastTurn",
    "DoublePowerIfTargetActed",
    "DoublePowerIfTargetAsleepCureTarget",
    "DoublePowerIfTargetHPLessThanHalf",
    "DoublePowerIfTargetInSky",
    "DoublePowerIfTargetLostHPThisTurn",
    "DoublePowerIfTargetNotActed",
    "DoublePowerIfTargetParalyzedCureTarget",
    "DoublePowerIfTargetPoisoned",
    "DoublePowerIfTargetStatusProblem",
    "DoublePowerIfTargetUnderground",
    "DoublePowerIfTargetUnderwater",
    "DoublePowerIfUserHasNoItem",
    "DoublePowerIfUserLastMoveFailed",
    "DoublePowerIfUserLostHPThisTurn",
    "DoublePowerIfUserPoisonedBurnedParalyzed",
    "DoublePowerIfUserStatsLoweredThisTurn",
    "DoublePowerInElectricTerrain",
    "EffectDependsOnEnvironment",
    "EffectivenessIncludesFlyingType",
    "EnsureNextCriticalHit",
    "EnsureNextMoveAlwaysHits",
    "FailsIfNotUserFirstTurn",
    "FailsIfTargetActed",
    "FailsIfTargetHasNoItem",
    "FailsIfUserDamagedThisTurn",
    "FailsIfUserHasUnusedMove",
    "FailsIfUserNotConsumedBerry",
    "FailsUnlessTargetSharesTypeWithUser",
    "FirePledge",
    "FixedDamage20",
    "FixedDamage40",
    "FixedDamageHalfTargetHP",
    "FixedDamageUserLevel",
    "FixedDamageUserLevelRandom",
    "FlinchTarget",
    "FlinchTargetDoublePowerIfTargetInSky",
    "FlinchTargetFailsIfNotUserFirstTurn",
    "FlinchTargetFailsIfUserNotAsleep",
    "FreezeFlinchTarget",
    "FreezeTarget",
    "FreezeTargetAlwaysHitsInHail",
    "FreezeTargetSuperEffectiveAgainstWater",
    "GiveUserStatusToTarget",
    "GrassPledge",
    "HealAllyOrDamageFoe",
    "HealTargetDependingOnGrassyTerrain",
    "HealTargetHalfOfTotalHP",
    "HealUserAndAlliesQuarterOfTotalHP",
    "HealUserAndAlliesQuarterOfTotalHPCureStatus",
    "HealUserByHalfOfDamageDone",
    "HealUserByHalfOfDamageDoneIfTargetAsleep",
    "HealUserByTargetAttackLowerTargetAttack1",
    "HealUserByThreeQuartersOfDamageDone",
    "HealUserDependingOnSandstorm",
    "HealUserDependingOnUserStockpile",
    "HealUserDependingOnWeather",
    "HealUserFullyAndFallAsleep",
    "HealUserHalfOfTotalHP",
    "HealUserHalfOfTotalHPLoseFlyingTypeThisTurn",
    "HealUserPositionNextTurn",
    "HigherPriorityInGrassyTerrain",
    "HitOncePerUserTeamMember",
    "HitThreeTimesAlwaysCriticalHit",
    "HitThreeTimesPowersUpWithEachHit",
    "HitTwoTimes",
    "HitTwoTimesFlinchTarget",
    "HitTwoTimesPoisonTarget",
    "HitTwoTimesTargetThenTargetAlly",
    "HitTwoToFiveTimes",
    "HitTwoToFiveTimesOrThreeForAshGreninja",
    "HitTwoToFiveTimesRaiseUserSpd1LowerUserDef1",
    "HitsAllFoesAndPowersUpInPsychicTerrain",
    "HitsTargetInSky",
    "HitsTargetInSkyGroundsTarget",
    "HoopaRemoveProtectionsBypassSubstituteLowerUserDef1",
    "IgnoreTargetAbility",
    "IgnoreTargetDefSpDefEvaStatStages",
    "InvertTargetStatStages",
    "LowerPPOfTargetLastMoveBy3",
    "LowerPPOfTargetLastMoveBy4",
    "LowerPoisonedTargetAtkSpAtkSpd1",
    "LowerTargetAccuracy1",
    "LowerTargetAtkDef1",
    "LowerTargetAtkSpAtk1",
    "LowerTargetAtkSpAtk1SwitchOutUser",
    "LowerTargetAttack1",
    "LowerTargetAttack1BypassSubstitute",
    "LowerTargetAttack2",
    "LowerTargetDefense1",
    "LowerTargetDefense1PowersUpInGravity",
    "LowerTargetDefense2",
    "LowerTargetEvasion1RemoveSideEffects",
    "LowerTargetEvasion2",
    "LowerTargetHPToUserHP",
    "LowerTargetSpAtk1",
    "LowerTargetSpAtk2",
    "LowerTargetSpAtk2IfCanAttract",
    "LowerTargetSpDef1",
    "LowerTargetSpDef2",
    "LowerTargetSpeed1",
    "LowerTargetSpeed1MakeTargetWeakerToFire",
    "LowerTargetSpeed1WeakerInGrassyTerrain",
    "LowerTargetSpeed2",
    "LowerUserAtkDef1",
    "LowerUserDefSpDef1",
    "LowerUserDefSpDef1RaiseUserAtkSpAtkSpd2",
    "LowerUserDefSpDefSpd1",
    "LowerUserDefense1",
    "LowerUserSpAtk2",
    "LowerUserSpeed1",
    "MaxUserAttackLoseHalfOfTotalHP",
    "MultiTurnAttackBideThenReturnDoubleDamage",
    "MultiTurnAttackConfuseUserAtEnd",
    "MultiTurnAttackPowersUpEachTurn",
    "MultiTurnAttackPreventSleeping",
    "NegateTargetAbility",
    "NegateTargetAbilityIfTargetActed",
    "NormalMovesBecomeElectric",
    "OHKO",
    "OHKOHitsUndergroundTarget",
    "OHKOIce",
    "ParalyzeBurnOrFreezeTarget",
    "ParalyzeFlinchTarget",
    "ParalyzeTarget",
    "ParalyzeTargetAlwaysHitsInRainHitsTargetInSky",
    "ParalyzeTargetIfNotTypeImmune",
    "PoisonTarget",
    "PoisonTargetLowerTargetSpeed1",
    "PowerDependsOnUserStockpile",
    "PowerHigherWithConsecutiveUse",
    "PowerHigherWithConsecutiveUseOnUserSide",
    "PowerHigherWithLessPP",
    "PowerHigherWithTargetFasterThanUser",
    "PowerHigherWithTargetHP",
    "PowerHigherWithTargetPositiveStatStages",
    "PowerHigherWithTargetWeight",
    "PowerHigherWithUserFasterThanTarget",
    "PowerHigherWithUserHP",
    "PowerHigherWithUserHappiness",
    "PowerHigherWithUserHeavierThanTarget",
    "PowerHigherWithUserPositiveStatStages",
    "PowerLowerWithUserHP",
    "PowerLowerWithUserHappiness",
    "PowerUpAllyMove",
    "ProtectUser",
    "ProtectUserBanefulBunker",
    "ProtectUserFromDamagingMovesKingsShield",
    "ProtectUserFromDamagingMovesObstruct",
    "ProtectUserFromTargetingMovesSpikyShield",
    "ProtectUserSideFromDamagingMovesIfUserFirstTurn",
    "ProtectUserSideFromMultiTargetDamagingMoves",
    "ProtectUserSideFromPriorityMoves",
    "ProtectUserSideFromStatusMoves",
    "PursueSwitchingFoe",
    "RaiseAlliesAtkDef1",
    "RaiseGrassBattlersDef1",
    "RaiseGroundedGrassBattlersAtkSpAtk1",
    "RaisePlusMinusUserAndAlliesAtkSpAtk1",
    "RaisePlusMinusUserAndAlliesDefSpDef1",
    "RaiseTargetAtkSpAtk2",
    "RaiseTargetAttack1",
    "RaiseTargetAttack2ConfuseTarget",
    "RaiseTargetRandomStat2",
    "RaiseTargetSpAtk1ConfuseTarget",
    "RaiseTargetSpDef1",
    "RaiseUserAtk1Spd2",
    "RaiseUserAtkAcc1",
    "RaiseUserAtkDef1",
    "RaiseUserAtkDefAcc1",
    "RaiseUserAtkSpAtk1",
    "RaiseUserAtkSpAtk1Or2InSun",
    "RaiseUserAtkSpd1",
    "RaiseUserAttack1",
    "RaiseUserAttack2",
    "RaiseUserAttack3IfTargetFaints",
    "RaiseUserCriticalHitRate2",
    "RaiseUserDefSpDef1",
    "RaiseUserDefense1",
    "RaiseUserDefense1CurlUpUser",
    "RaiseUserDefense2",
    "RaiseUserDefense3",
    "RaiseUserEvasion1",
    "RaiseUserEvasion2MinimizeUser",
    "RaiseUserMainStats1",
    "RaiseUserMainStats1LoseThirdOfTotalHP",
    "RaiseUserMainStats1TrapUserInBattle",
    "RaiseUserSpAtk1",
    "RaiseUserSpAtk2",
    "RaiseUserSpAtk3",
    "RaiseUserSpAtkSpDef1",
    "RaiseUserSpAtkSpDefSpd1",
    "RaiseUserSpDef1PowerUpElectricMove",
    "RaiseUserSpDef2",
    "RaiseUserSpeed1",
    "RaiseUserSpeed2",
    "RaiseUserSpeed2LowerUserWeight",
    "RandomPowerDoublePowerIfTargetUnderground",
    "RandomlyDamageOrHealTarget",
    "RecoilHalfOfDamageDealt",
    "RecoilQuarterOfDamageDealt",
    "RecoilThirdOfDamageDealt",
    "RecoilThirdOfDamageDealtBurnTarget",
    "RecoilThirdOfDamageDealtParalyzeTarget",
    "RedirectAllMovesToTarget",
    "RedirectAllMovesToUser",
    "RemoveProtections",
    "RemoveProtectionsBypassSubstitute",
    "RemoveScreens",
    "RemoveTargetItem",
    "RemoveTerrain",
    "RemoveUserBindingAndEntryHazards",
    "ReplaceMoveThisBattleWithTargetLastMoveUsed",
    "ReplaceMoveWithTargetLastMoveUsed",
    "ResetAllBattlersStatStages",
    "ResetTargetStatStages",
    "RestoreUserConsumedItem",
    "SetAttackerMovePPTo0IfUserFaints",
    "SetTargetAbilityToInsomnia",
    "SetTargetAbilityToSimple",
    "SetTargetAbilityToUserAbility",
    "SetTargetTypesToPsychic",
    "SetTargetTypesToWater",
    "SetUserAbilityToTargetAbility",
    "SetUserTypesBasedOnEnvironment",
    "SetUserTypesToResistLastAttack",
    "SetUserTypesToTargetTypes",
    "SetUserTypesToUserMoveType",
    "SleepTarget",
    "SleepTargetChangeUserMeloettaForm",
    "SleepTargetIfUserDarkrai",
    "SleepTargetNextTurn",
    "StartDamageTargetEachTurnIfTargetAsleep",
    "StartElectricTerrain",
    "StartGrassyTerrain",
    "StartGravity",
    "StartHailWeather",
    "StartHealUserEachTurn",
    "StartHealUserEachTurnTrapUserInBattle",
    "StartLeechSeedTarget",
    "StartMistyTerrain",
    "StartNegateHeldItems",
    "StartNegateTargetEvasionStatStageAndDarkImmunity",
    "StartNegateTargetEvasionStatStageAndGhostImmunity",
    "StartPerishCountsForAllBattlers",
    "StartPreventCriticalHitsAgainstUserSide",
    "StartPsychicTerrain",
    "StartRainWeather",
    "StartRaiseUserAtk1WhenDamaged",
    "StartSandstormWeather",
    "StartSlowerBattlersActFirst",
    "StartSunWeather",
    "StartSwapAllBattlersBaseDefensiveStats",
    "StartTargetAirborneAndAlwaysHitByMoves",
    "StartTargetCannotUseItem",
    "StartUserAirborne",
    "StartUserSideDoubleSpeed",
    "StartUserSideImmunityToInflictedStatus",
    "StartUserSideImmunityToStatStageLowering",
    "StartWeakenDamageAgainstUserSideIfHail",
    "StartWeakenElectricMoves",
    "StartWeakenFireMoves",
    "StartWeakenPhysicalDamageAgainstUserSide",
    "StartWeakenSpecialDamageAgainstUserSide",
    "StealAndUseBeneficialStatusMove",
    "SwapSideEffects",
    "SwitchOutTargetDamagingMove",
    "SwitchOutTargetStatusMove",
    "SwitchOutUserDamagingMove",
    "SwitchOutUserPassOnEffects",
    "SwitchOutUserStatusMove",
    "TargetActsLast",
    "TargetActsNext",
    "TargetMovesBecomeElectric",
    "TargetNextFireMoveDamagesTarget",
    "TargetTakesUserItem",
    "TargetUsesItsLastUsedMoveAgain",
    "ThrowUserItemAtTarget",
    "TransformUserIntoTarget",
    "TrapAllBattlersInBattleForOneTurn",
    "TrapTargetInBattle",
    "TrapTargetInBattleLowerTargetDefSpDef1EachTurn",
    "TrapTargetInBattleMainEffect",
    "TrapUserAndTargetInBattle",
    "TwoTurnAttack",
    "TwoTurnAttackBurnTarget",
    "TwoTurnAttackChargeRaiseUserDefense1",
    "TwoTurnAttackChargeRaiseUserSpAtk1",
    "TwoTurnAttackFlinchTarget",
    "TwoTurnAttackInvulnerableInSky",
    "TwoTurnAttackInvulnerableInSkyParalyzeTarget",
    "TwoTurnAttackInvulnerableInSkyTargetCannotAct",
    "TwoTurnAttackInvulnerableRemoveProtections",
    "TwoTurnAttackInvulnerableUnderground",
    "TwoTurnAttackInvulnerableUnderwater",
    "TwoTurnAttackOneTurnInSun",
    "TwoTurnAttackParalyzeTarget",
    "TwoTurnAttackRaiseUserSpAtkSpDefSpd2",
    "TypeAndPowerDependOnTerrain",
    "TypeAndPowerDependOnUserBerry",
    "TypeAndPowerDependOnWeather",
    "TypeDependsOnUserDrive",
    "TypeDependsOnUserIVs",
    "TypeDependsOnUserMemory",
    "TypeDependsOnUserMorpekoFormRaiseUserSpeed1",
    "TypeDependsOnUserPlate",
    "TypeIsUserFirstType",
    "UseLastMoveUsed",
    "UseLastMoveUsedByTarget",
    "UseMoveDependingOnEnvironment",
    "UseMoveTargetIsAboutToUse",
    "UseRandomMove",
    "UseRandomMoveFromUserParty",
    "UseRandomUserMoveIfAsleep",
    "UseTargetAttackInsteadOfUserAttack",
    "UseTargetDefenseInsteadOfTargetSpDef",
    "UseUserDefenseInsteadOfUserAttack",
    "UsedAfterAllyRoundWithDoublePower",
    "UsedAfterUserTakesPhysicalDamage",
    "UserAddStockpileRaiseDefSpDef1",
    "UserConsumeBerryRaiseDefense2",
    "UserConsumeTargetBerry",
    "UserCopyTargetStatStages",
    "UserEnduresFaintingThisTurn",
    "UserFaintsExplosive",
    "UserFaintsFixedDamageUserHP",
    "UserFaintsHealAndCureReplacement",
    "UserFaintsHealAndCureReplacementRestorePP",
    "UserFaintsLowerTargetAtkSpAtk2",
    "UserFaintsPowersUpInMistyTerrainExplosive",
    "UserLosesFireType",
    "UserLosesHalfOfTotalHP",
    "UserLosesHalfOfTotalHPExplosive",
    "UserMakeSubstitute",
    "UserStealTargetPositiveStatStages",
    "UserSwapBaseAtkDef",
    "UserSwapsPositionsWithAlly",
    "UserTakesTargetItem",
    "UserTargetAverageBaseAtkSpAtk",
    "UserTargetAverageBaseDefSpDef",
    "UserTargetAverageHP",
    "UserTargetSwapAbilities",
    "UserTargetSwapAtkSpAtkStages",
    "UserTargetSwapBaseSpeed",
    "UserTargetSwapDefSpDefStages",
    "UserTargetSwapItems",
    "UserTargetSwapStatStages",
    "WaterPledge"
};
} // namespace

std::string MoveDef::CategoryToString(Category e)
{
    switch (e)
    {
    case PHYSICAL:
        return "PHYSICAL";
    case SPECIAL:
        return "SPECIAL";
    case STATUS:
        return "STATUS";
    case __SIZE:
        return "__SIZE";
    }
    return "???";
}

MoveDef::Category MoveDef::CategoryFromString(std::string e)
{
    std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c) { return std::toupper(c); });

    if (e == "SPECIAL")
        return SPECIAL;
    else if (e == "STATUS")
        return STATUS;
    else
        return PHYSICAL;
}

std::string MoveDef::FunctionCodeToString(FunctionCode e)
{
    size_t index = static_cast<size_t>(e);
    if (index >= FunctionCodeCount)
        return "???";
    return functionCodeNames[index];
}

MoveDef::FunctionCode MoveDef::FunctionCodeFromString(std::string e)
{
    for (size_t i = 0; i < FunctionCodeCount; ++i)
    {
        if (e == functionCodeNames[i])
            return static_cast<FunctionCode>(i);
    }
    return FunctionCode::None;
}


std::string MoveDef::TargetToString(Target e)
{
    static char const* names[] = {
        "None", "AllAllies", "AllBattlers", "AllNearFoes", "AllNearOthers", "BothSides", "FoeSide",
        "NearAlly", "NearFoe", "NearOther", "Other", "RandomNearFoe", "User", "UserAndAllies",
        "UserOrNearAlly", "UserSide"
    };
    size_t index = static_cast<size_t>(e);
    if (index >= TargetCount)
        return "???";
    return names[index];
}

MoveDef::Target MoveDef::TargetFromString(std::string e)
{
    for (size_t i = 0; i < TargetCount; ++i)
    {
        if (e == TargetToString(static_cast<Target>(i)))
            return static_cast<Target>(i);
    }
    return Target::NearOther;
}

std::string MoveDef::FlagToString(Flag e)
{
    static char const* names[] = {
        "Contact", "CanProtect", "CanMirrorMove", "Sound", "Punching", "Biting", "Bomb", "Pulse",
        "Dance", "Powder", "HighCriticalHitRate", "ThawsUser", "TramplesMinimize", "CannotMetronome"
    };
    size_t index = static_cast<size_t>(e);
    if (index >= FlagCount)
        return "???";
    return names[index];
}

MoveDef::Flag MoveDef::FlagFromString(std::string e)
{
    for (size_t i = 0; i < FlagCount; ++i)
    {
        if (e == FlagToString(static_cast<Flag>(i)))
            return static_cast<Flag>(i);
    }
    return Flag::__SIZE; // invalid
}

MoveDef::MoveDef()
{
}

MoveDef::MoveDefPtr MoveDef::fromPropertyTree(std::string const& id, pt::ptree const& pt)
{
    auto pkmn      = std::make_shared<MoveDef>();
    pkmn->id       = id;
    pkmn->name     = pt.get<std::string>("Name");
    pkmn->type     = pt.get<std::string>("Type");
    pkmn->accuracy = pt.get<size_t>("Accuracy");
    if (pt.count("Power"))
        pkmn->power = pt.get<size_t>("Power");
    pkmn->totalPP  = pt.get<size_t>("TotalPP");
    pkmn->category = CategoryFromString(pt.get<std::string>("Category"));
    if (pt.count("Priority"))
        pkmn->priority = pt.get<int>("Priority");
    if (pt.count("FunctionCode"))
        pkmn->functionCode = FunctionCodeFromString(pt.get<std::string>("FunctionCode"));
    if (pt.count("EffectChance"))
        pkmn->effectChance = pt.get<size_t>("EffectChance");
    if (pt.count("Target"))
        pkmn->target = TargetFromString(pt.get<std::string>("Target"));
    if (pt.count("Flags"))
    {
        std::string flagsStr = pt.get<std::string>("Flags");
        size_t      start    = 0;
        while (start < flagsStr.size())
        {
            size_t comma = flagsStr.find(',', start);
            if (comma == std::string::npos)
                comma = flagsStr.size();
            std::string token = flagsStr.substr(start, comma - start);
            while (!token.empty() && token.front() == ' ')
                token.erase(token.begin());
            while (!token.empty() && token.back() == ' ')
                token.pop_back();
            if (!token.empty())
            {
                Flag f = FlagFromString(token);
                if (f != Flag::__SIZE)
                    pkmn->flags.push_back(f);
            }
            start = comma + 1;
        }
    }
    if (pt.count("Description"))
        pkmn->description = pt.get<std::string>("Description");

    return pkmn;
}

std::vector<MoveDef::MoveDefPtr> MoveDef::vectorFromPropertyTree(pt::ptree const& pt)
{
    std::vector<MoveDefPtr> pkmns;
    for (auto pos = pt.begin(); pos != pt.end(); ++pos)
    {
        auto p = *pos;
        pkmns.push_back(fromPropertyTree(p.first, p.second));
    }
    return pkmns;
}

std::string MoveDef::getId() const
{
    return id;
}

void MoveDef::setId(std::string const& newId)
{
    id = newId;
}

std::string MoveDef::getName() const
{
    return name;
}

void MoveDef::setName(std::string const& newName)
{
    name = newName;
}

std::string MoveDef::getType() const
{
    return type;
}

void MoveDef::setType(std::string const& newType)
{
    type = newType;
}

size_t MoveDef::getAccuracy() const
{
    return accuracy;
}

void MoveDef::setAccuracy(size_t newAccuracy)
{
    accuracy = newAccuracy;
}

size_t MoveDef::getPower() const
{
    return power;
}

void MoveDef::setPower(size_t newPower)
{
    power = newPower;
}

size_t MoveDef::getTotalPP() const
{
    return totalPP;
}

void MoveDef::setTotalPP(size_t newTotalPP)
{
    totalPP = newTotalPP;
}

MoveDef::Category MoveDef::getCategory() const
{
    return category;
}

void MoveDef::setCategory(Category newCategory)
{
    category = newCategory;
}

MoveDef::Category MoveDef::battleCategory() const
{
    if (category == STATUS || power == 0)
        return STATUS;
    if (type == "FIRE" || type == "WATER" || type == "GRASS" || type == "ELECTRIC" || type == "PSYCHIC"
        || type == "ICE" || type == "DRAGON" || type == "DARK")
        return SPECIAL;
    return PHYSICAL;
}

int MoveDef::getPriority() const
{
    return priority;
}

void MoveDef::setPriority(int newPriority)
{
    priority = newPriority;
}

MoveDef::FunctionCode MoveDef::getFunctionCode() const
{
    return functionCode;
}

void MoveDef::setFunctionCode(FunctionCode newFunctionCode)
{
    functionCode = newFunctionCode;
}

size_t MoveDef::getEffectChance() const
{
    return effectChance;
}

void MoveDef::setEffectChance(size_t newEffectChance)
{
    effectChance = newEffectChance;
}

MoveDef::Target MoveDef::getTarget() const
{
    return target;
}

void MoveDef::setTarget(Target newTarget)
{
    target = newTarget;
}

std::vector<MoveDef::Flag> const& MoveDef::getFlags() const
{
    return flags;
}

void MoveDef::setFlags(std::vector<Flag> const& newFlags)
{
    flags = newFlags;
}

bool MoveDef::hasFlag(Flag flag) const
{
    return std::find(flags.begin(), flags.end(), flag) != flags.end();
}

std::string MoveDef::getDescription() const
{
    return description;
}

void MoveDef::setDescription(std::string const& newDescription)
{
    description = newDescription;
}

void tag_invoke(js::value_from_tag, js::value& jv, MoveDef::MoveDefPtr const& o)
{
    if (o && !o->id.empty())
    {
        js::array jsFlags;
        for (auto const& f : o->flags)
            jsFlags.push_back(js::value(MoveDef::FlagToString(f)));
        jv = {
            {"id",           o->id                             },
            {"name",         o->name                           },
            {"type",         o->type                           },
            {"accuracy",     o->accuracy                       },
            {"power",        o->power                          },
            {"totalPP",      o->totalPP                        },
            {"category",     static_cast<int>(o->category)     },
            {"priority",     o->priority                       },
            {"functionCode", static_cast<int>(o->functionCode) },
            {"effectChance", o->effectChance                   },
            {"target",       static_cast<int>(o->target)       },
            {"flags",        jsFlags                           },
            {"description",  o->description                    }
        };
    }
    else
    {
        jv = {};
    }
}

MoveDef::MoveDefPtr tag_invoke(js::value_to_tag<MoveDef::MoveDefPtr>, js::value const& jv)
{
    js::object const& obj = jv.as_object();
    if (obj.empty())
        return nullptr;

    std::string id = js::value_to<std::string>(obj.at("id"));
    if (id.empty())
        return nullptr;
    else
    {
        auto pkmn      = std::make_shared<MoveDef>();
        pkmn->id       = id;
        pkmn->name     = js::value_to<std::string>(obj.at("name"));
        pkmn->type     = js::value_to<std::string>(obj.at("type"));
        pkmn->accuracy = js::value_to<size_t>(obj.at("accuracy"));
        pkmn->power    = js::value_to<size_t>(obj.at("power"));
        pkmn->totalPP  = js::value_to<size_t>(obj.at("totalPP"));
        pkmn->category = static_cast<MoveDef::Category>(js::value_to<int>(obj.at("category")));
        pkmn->priority = js::value_to<int>(obj.at("priority"));
        if (auto const* fc = obj.if_contains("functionCode"))
        {
            if (fc->is_string())
                pkmn->functionCode = MoveDef::FunctionCodeFromString(js::value_to<std::string>(*fc));
            else
                pkmn->functionCode = static_cast<MoveDef::FunctionCode>(js::value_to<int>(*fc));
        }
        if (auto const* ec = obj.if_contains("effectChance"))
            pkmn->effectChance = js::value_to<size_t>(*ec);
        if (auto const* tg = obj.if_contains("target"))
        {
            if (tg->is_string())
                pkmn->target = MoveDef::TargetFromString(js::value_to<std::string>(*tg));
            else
                pkmn->target = static_cast<MoveDef::Target>(js::value_to<int>(*tg));
        }
        if (auto const* fl = obj.if_contains("flags"))
        {
            for (auto const& value : fl->as_array())
            {
                MoveDef::Flag f = MoveDef::FlagFromString(js::value_to<std::string>(value));
                if (f != MoveDef::Flag::__SIZE)
                    pkmn->flags.push_back(f);
            }
        }
        if (auto const* desc = obj.if_contains("description"))
            pkmn->description = js::value_to<std::string>(*desc);
        return pkmn;
    }
}
