#ifndef MOVEDEF_H
#define MOVEDEF_H

#include <boost/json.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <memory>
#include <string>
#include <vector>

namespace js = boost::json;
namespace pt = boost::property_tree;

class MoveDef
{
public:
    enum Category
    {
        PHYSICAL,
        SPECIAL,
        STATUS,
        __SIZE
    };


    enum class Target
    {
        None,
        AllAllies,
        AllBattlers,
        AllNearFoes,
        AllNearOthers,
        BothSides,
        FoeSide,
        NearAlly,
        NearFoe,
        NearOther,
        Other,
        RandomNearFoe,
        User,
        UserAndAllies,
        UserOrNearAlly,
        UserSide,
        __SIZE
    };

    enum class Flag
    {
        Contact,
        CanProtect,
        CanMirrorMove,
        Sound,
        Punching,
        Biting,
        Bomb,
        Pulse,
        Dance,
        Powder,
        HighCriticalHitRate,
        ThawsUser,
        TramplesMinimize,
        CannotMetronome,
        __SIZE
    };

    enum class FunctionCode
    {
        None,
        AddGhostTypeToTarget,
        AddGrassTypeToTarget,
        AddMoneyGainedFromBattle,
        AddSpikesToFoeSide,
        AddStealthRocksToFoeSide,
        AddStickyWebToFoeSide,
        AddToxicSpikesToFoeSide,
        AllBattlersConsumeBerry,
        AlwaysCriticalHit,
        AttackAndSkipNextTurn,
        AttackTwoTurnsLater,
        AttackerFaintsIfUserFaints,
        AttractTarget,
        BadPoisonTarget,
        BindTarget,
        BindTargetDoublePowerIfTargetUnderwater,
        BounceBackProblemCausingStatusMoves,
        BurnAttackerBeforeUserActs,
        BurnFlinchTarget,
        BurnTarget,
        BurnTargetIfTargetStatsRaisedThisTurn,
        CannotBeRedirected,
        CannotMakeTargetFaint,
        CategoryDependsOnHigherDamageIgnoreTargetAbility,
        CategoryDependsOnHigherDamagePoisonTarget,
        ConfuseTarget,
        ConfuseTargetAlwaysHitsInRainHitsTargetInSky,
        CorrodeTargetItem,
        CounterDamagePlusHalf,
        CounterPhysicalDamage,
        CounterSpecialDamage,
        CrashDamageIfFailsUnusableInGravity,
        CureTargetBurn,
        CureTargetStatusHealUserHalfOfTotalHP,
        CureUserBurnPoisonParalysis,
        CureUserPartyStatus,
        CurseTargetOrLowerUserSpd1RaiseUserAtkDef1,
        DamageTargetAlly,
        DestroyTargetBerryOrGem,
        DisableTargetHealingMoves,
        DisableTargetLastMoveUsed,
        DisableTargetMovesKnownByUser,
        DisableTargetSoundMoves,
        DisableTargetStatusMoves,
        DisableTargetUsingDifferentMove,
        DisableTargetUsingSameMoveConsecutively,
        DoesNothingCongratulations,
        DoesNothingFailsIfNoAlly,
        DoesNothingUnusableInGravity,
        DoubleMoneyGainedFromBattle,
        DoublePowerAfterFusionBolt,
        DoublePowerAfterFusionFlare,
        DoublePowerIfAllyFaintedLastTurn,
        DoublePowerIfTargetActed,
        DoublePowerIfTargetAsleepCureTarget,
        DoublePowerIfTargetHPLessThanHalf,
        DoublePowerIfTargetInSky,
        DoublePowerIfTargetLostHPThisTurn,
        DoublePowerIfTargetNotActed,
        DoublePowerIfTargetParalyzedCureTarget,
        DoublePowerIfTargetPoisoned,
        DoublePowerIfTargetStatusProblem,
        DoublePowerIfTargetUnderground,
        DoublePowerIfTargetUnderwater,
        DoublePowerIfUserHasNoItem,
        DoublePowerIfUserLastMoveFailed,
        DoublePowerIfUserLostHPThisTurn,
        DoublePowerIfUserPoisonedBurnedParalyzed,
        DoublePowerIfUserStatsLoweredThisTurn,
        DoublePowerInElectricTerrain,
        EffectDependsOnEnvironment,
        EffectivenessIncludesFlyingType,
        EnsureNextCriticalHit,
        EnsureNextMoveAlwaysHits,
        FailsIfNotUserFirstTurn,
        FailsIfTargetActed,
        FailsIfTargetHasNoItem,
        FailsIfUserDamagedThisTurn,
        FailsIfUserHasUnusedMove,
        FailsIfUserNotConsumedBerry,
        FailsUnlessTargetSharesTypeWithUser,
        FirePledge,
        FixedDamage20,
        FixedDamage40,
        FixedDamageHalfTargetHP,
        FixedDamageUserLevel,
        FixedDamageUserLevelRandom,
        FlinchTarget,
        FlinchTargetDoublePowerIfTargetInSky,
        FlinchTargetFailsIfNotUserFirstTurn,
        FlinchTargetFailsIfUserNotAsleep,
        FreezeFlinchTarget,
        FreezeTarget,
        FreezeTargetAlwaysHitsInHail,
        FreezeTargetSuperEffectiveAgainstWater,
        GiveUserStatusToTarget,
        GrassPledge,
        HealAllyOrDamageFoe,
        HealTargetDependingOnGrassyTerrain,
        HealTargetHalfOfTotalHP,
        HealUserAndAlliesQuarterOfTotalHP,
        HealUserAndAlliesQuarterOfTotalHPCureStatus,
        HealUserByHalfOfDamageDone,
        HealUserByHalfOfDamageDoneIfTargetAsleep,
        HealUserByTargetAttackLowerTargetAttack1,
        HealUserByThreeQuartersOfDamageDone,
        HealUserDependingOnSandstorm,
        HealUserDependingOnUserStockpile,
        HealUserDependingOnWeather,
        HealUserFullyAndFallAsleep,
        HealUserHalfOfTotalHP,
        HealUserHalfOfTotalHPLoseFlyingTypeThisTurn,
        HealUserPositionNextTurn,
        HigherPriorityInGrassyTerrain,
        HitOncePerUserTeamMember,
        HitThreeTimesAlwaysCriticalHit,
        HitThreeTimesPowersUpWithEachHit,
        HitTwoTimes,
        HitTwoTimesFlinchTarget,
        HitTwoTimesPoisonTarget,
        HitTwoTimesTargetThenTargetAlly,
        HitTwoToFiveTimes,
        HitTwoToFiveTimesOrThreeForAshGreninja,
        HitTwoToFiveTimesRaiseUserSpd1LowerUserDef1,
        HitsAllFoesAndPowersUpInPsychicTerrain,
        HitsTargetInSky,
        HitsTargetInSkyGroundsTarget,
        HoopaRemoveProtectionsBypassSubstituteLowerUserDef1,
        IgnoreTargetAbility,
        IgnoreTargetDefSpDefEvaStatStages,
        InvertTargetStatStages,
        LowerPPOfTargetLastMoveBy3,
        LowerPPOfTargetLastMoveBy4,
        LowerPoisonedTargetAtkSpAtkSpd1,
        LowerTargetAccuracy1,
        LowerTargetAtkDef1,
        LowerTargetAtkSpAtk1,
        LowerTargetAtkSpAtk1SwitchOutUser,
        LowerTargetAttack1,
        LowerTargetAttack1BypassSubstitute,
        LowerTargetAttack2,
        LowerTargetDefense1,
        LowerTargetDefense1PowersUpInGravity,
        LowerTargetDefense2,
        LowerTargetEvasion1RemoveSideEffects,
        LowerTargetEvasion2,
        LowerTargetHPToUserHP,
        LowerTargetSpAtk1,
        LowerTargetSpAtk2,
        LowerTargetSpAtk2IfCanAttract,
        LowerTargetSpDef1,
        LowerTargetSpDef2,
        LowerTargetSpeed1,
        LowerTargetSpeed1MakeTargetWeakerToFire,
        LowerTargetSpeed1WeakerInGrassyTerrain,
        LowerTargetSpeed2,
        LowerUserAtkDef1,
        LowerUserDefSpDef1,
        LowerUserDefSpDef1RaiseUserAtkSpAtkSpd2,
        LowerUserDefSpDefSpd1,
        LowerUserDefense1,
        LowerUserSpAtk2,
        LowerUserSpeed1,
        MaxUserAttackLoseHalfOfTotalHP,
        MultiTurnAttackBideThenReturnDoubleDamage,
        MultiTurnAttackConfuseUserAtEnd,
        MultiTurnAttackPowersUpEachTurn,
        MultiTurnAttackPreventSleeping,
        NegateTargetAbility,
        NegateTargetAbilityIfTargetActed,
        NormalMovesBecomeElectric,
        OHKO,
        OHKOHitsUndergroundTarget,
        OHKOIce,
        ParalyzeBurnOrFreezeTarget,
        ParalyzeFlinchTarget,
        ParalyzeTarget,
        ParalyzeTargetAlwaysHitsInRainHitsTargetInSky,
        ParalyzeTargetIfNotTypeImmune,
        PoisonTarget,
        PoisonTargetLowerTargetSpeed1,
        PowerDependsOnUserStockpile,
        PowerHigherWithConsecutiveUse,
        PowerHigherWithConsecutiveUseOnUserSide,
        PowerHigherWithLessPP,
        PowerHigherWithTargetFasterThanUser,
        PowerHigherWithTargetHP,
        PowerHigherWithTargetPositiveStatStages,
        PowerHigherWithTargetWeight,
        PowerHigherWithUserFasterThanTarget,
        PowerHigherWithUserHP,
        PowerHigherWithUserHappiness,
        PowerHigherWithUserHeavierThanTarget,
        PowerHigherWithUserPositiveStatStages,
        PowerLowerWithUserHP,
        PowerLowerWithUserHappiness,
        PowerUpAllyMove,
        ProtectUser,
        ProtectUserBanefulBunker,
        ProtectUserFromDamagingMovesKingsShield,
        ProtectUserFromDamagingMovesObstruct,
        ProtectUserFromTargetingMovesSpikyShield,
        ProtectUserSideFromDamagingMovesIfUserFirstTurn,
        ProtectUserSideFromMultiTargetDamagingMoves,
        ProtectUserSideFromPriorityMoves,
        ProtectUserSideFromStatusMoves,
        PursueSwitchingFoe,
        RaiseAlliesAtkDef1,
        RaiseGrassBattlersDef1,
        RaiseGroundedGrassBattlersAtkSpAtk1,
        RaisePlusMinusUserAndAlliesAtkSpAtk1,
        RaisePlusMinusUserAndAlliesDefSpDef1,
        RaiseTargetAtkSpAtk2,
        RaiseTargetAttack1,
        RaiseTargetAttack2ConfuseTarget,
        RaiseTargetRandomStat2,
        RaiseTargetSpAtk1ConfuseTarget,
        RaiseTargetSpDef1,
        RaiseUserAtk1Spd2,
        RaiseUserAtkAcc1,
        RaiseUserAtkDef1,
        RaiseUserAtkDefAcc1,
        RaiseUserAtkSpAtk1,
        RaiseUserAtkSpAtk1Or2InSun,
        RaiseUserAtkSpd1,
        RaiseUserAttack1,
        RaiseUserAttack2,
        RaiseUserAttack3IfTargetFaints,
        RaiseUserCriticalHitRate2,
        RaiseUserDefSpDef1,
        RaiseUserDefense1,
        RaiseUserDefense1CurlUpUser,
        RaiseUserDefense2,
        RaiseUserDefense3,
        RaiseUserEvasion1,
        RaiseUserEvasion2MinimizeUser,
        RaiseUserMainStats1,
        RaiseUserMainStats1LoseThirdOfTotalHP,
        RaiseUserMainStats1TrapUserInBattle,
        RaiseUserSpAtk1,
        RaiseUserSpAtk2,
        RaiseUserSpAtk3,
        RaiseUserSpAtkSpDef1,
        RaiseUserSpAtkSpDefSpd1,
        RaiseUserSpDef1PowerUpElectricMove,
        RaiseUserSpDef2,
        RaiseUserSpeed1,
        RaiseUserSpeed2,
        RaiseUserSpeed2LowerUserWeight,
        RandomPowerDoublePowerIfTargetUnderground,
        RandomlyDamageOrHealTarget,
        RecoilHalfOfDamageDealt,
        RecoilQuarterOfDamageDealt,
        RecoilThirdOfDamageDealt,
        RecoilThirdOfDamageDealtBurnTarget,
        RecoilThirdOfDamageDealtParalyzeTarget,
        RedirectAllMovesToTarget,
        RedirectAllMovesToUser,
        RemoveProtections,
        RemoveProtectionsBypassSubstitute,
        RemoveScreens,
        RemoveTargetItem,
        RemoveTerrain,
        RemoveUserBindingAndEntryHazards,
        ReplaceMoveThisBattleWithTargetLastMoveUsed,
        ReplaceMoveWithTargetLastMoveUsed,
        ResetAllBattlersStatStages,
        ResetTargetStatStages,
        RestoreUserConsumedItem,
        SetAttackerMovePPTo0IfUserFaints,
        SetTargetAbilityToInsomnia,
        SetTargetAbilityToSimple,
        SetTargetAbilityToUserAbility,
        SetTargetTypesToPsychic,
        SetTargetTypesToWater,
        SetUserAbilityToTargetAbility,
        SetUserTypesBasedOnEnvironment,
        SetUserTypesToResistLastAttack,
        SetUserTypesToTargetTypes,
        SetUserTypesToUserMoveType,
        SleepTarget,
        SleepTargetChangeUserMeloettaForm,
        SleepTargetIfUserDarkrai,
        SleepTargetNextTurn,
        StartDamageTargetEachTurnIfTargetAsleep,
        StartElectricTerrain,
        StartGrassyTerrain,
        StartGravity,
        StartHailWeather,
        StartHealUserEachTurn,
        StartHealUserEachTurnTrapUserInBattle,
        StartLeechSeedTarget,
        StartMistyTerrain,
        StartNegateHeldItems,
        StartNegateTargetEvasionStatStageAndDarkImmunity,
        StartNegateTargetEvasionStatStageAndGhostImmunity,
        StartPerishCountsForAllBattlers,
        StartPreventCriticalHitsAgainstUserSide,
        StartPsychicTerrain,
        StartRainWeather,
        StartRaiseUserAtk1WhenDamaged,
        StartSandstormWeather,
        StartSlowerBattlersActFirst,
        StartSunWeather,
        StartSwapAllBattlersBaseDefensiveStats,
        StartTargetAirborneAndAlwaysHitByMoves,
        StartTargetCannotUseItem,
        StartUserAirborne,
        StartUserSideDoubleSpeed,
        StartUserSideImmunityToInflictedStatus,
        StartUserSideImmunityToStatStageLowering,
        StartWeakenDamageAgainstUserSideIfHail,
        StartWeakenElectricMoves,
        StartWeakenFireMoves,
        StartWeakenPhysicalDamageAgainstUserSide,
        StartWeakenSpecialDamageAgainstUserSide,
        StealAndUseBeneficialStatusMove,
        SwapSideEffects,
        SwitchOutTargetDamagingMove,
        SwitchOutTargetStatusMove,
        SwitchOutUserDamagingMove,
        SwitchOutUserPassOnEffects,
        SwitchOutUserStatusMove,
        TargetActsLast,
        TargetActsNext,
        TargetMovesBecomeElectric,
        TargetNextFireMoveDamagesTarget,
        TargetTakesUserItem,
        TargetUsesItsLastUsedMoveAgain,
        ThrowUserItemAtTarget,
        TransformUserIntoTarget,
        TrapAllBattlersInBattleForOneTurn,
        TrapTargetInBattle,
        TrapTargetInBattleLowerTargetDefSpDef1EachTurn,
        TrapTargetInBattleMainEffect,
        TrapUserAndTargetInBattle,
        TwoTurnAttack,
        TwoTurnAttackBurnTarget,
        TwoTurnAttackChargeRaiseUserDefense1,
        TwoTurnAttackChargeRaiseUserSpAtk1,
        TwoTurnAttackFlinchTarget,
        TwoTurnAttackInvulnerableInSky,
        TwoTurnAttackInvulnerableInSkyParalyzeTarget,
        TwoTurnAttackInvulnerableInSkyTargetCannotAct,
        TwoTurnAttackInvulnerableRemoveProtections,
        TwoTurnAttackInvulnerableUnderground,
        TwoTurnAttackInvulnerableUnderwater,
        TwoTurnAttackOneTurnInSun,
        TwoTurnAttackParalyzeTarget,
        TwoTurnAttackRaiseUserSpAtkSpDefSpd2,
        TypeAndPowerDependOnTerrain,
        TypeAndPowerDependOnUserBerry,
        TypeAndPowerDependOnWeather,
        TypeDependsOnUserDrive,
        TypeDependsOnUserIVs,
        TypeDependsOnUserMemory,
        TypeDependsOnUserMorpekoFormRaiseUserSpeed1,
        TypeDependsOnUserPlate,
        TypeIsUserFirstType,
        UseLastMoveUsed,
        UseLastMoveUsedByTarget,
        UseMoveDependingOnEnvironment,
        UseMoveTargetIsAboutToUse,
        UseRandomMove,
        UseRandomMoveFromUserParty,
        UseRandomUserMoveIfAsleep,
        UseTargetAttackInsteadOfUserAttack,
        UseTargetDefenseInsteadOfTargetSpDef,
        UseUserDefenseInsteadOfUserAttack,
        UsedAfterAllyRoundWithDoublePower,
        UsedAfterUserTakesPhysicalDamage,
        UserAddStockpileRaiseDefSpDef1,
        UserConsumeBerryRaiseDefense2,
        UserConsumeTargetBerry,
        UserCopyTargetStatStages,
        UserEnduresFaintingThisTurn,
        UserFaintsExplosive,
        UserFaintsFixedDamageUserHP,
        UserFaintsHealAndCureReplacement,
        UserFaintsHealAndCureReplacementRestorePP,
        UserFaintsLowerTargetAtkSpAtk2,
        UserFaintsPowersUpInMistyTerrainExplosive,
        UserLosesFireType,
        UserLosesHalfOfTotalHP,
        UserLosesHalfOfTotalHPExplosive,
        UserMakeSubstitute,
        UserStealTargetPositiveStatStages,
        UserSwapBaseAtkDef,
        UserSwapsPositionsWithAlly,
        UserTakesTargetItem,
        UserTargetAverageBaseAtkSpAtk,
        UserTargetAverageBaseDefSpDef,
        UserTargetAverageHP,
        UserTargetSwapAbilities,
        UserTargetSwapAtkSpAtkStages,
        UserTargetSwapBaseSpeed,
        UserTargetSwapDefSpDefStages,
        UserTargetSwapItems,
        UserTargetSwapStatStages,
        WaterPledge,
        __SIZE
    };

    static constexpr size_t CategoryCount     = Category::__SIZE;
    static constexpr size_t FunctionCodeCount = static_cast<size_t>(FunctionCode::__SIZE);

    static constexpr size_t TargetCount = static_cast<size_t>(Target::__SIZE);
    static std::string      TargetToString(Target e);
    static Target           TargetFromString(std::string e);

    static constexpr size_t FlagCount = static_cast<size_t>(Flag::__SIZE);
    static std::string      FlagToString(Flag e);
    static Flag             FlagFromString(std::string e);


    static std::string CategoryToString(Category e);
    static Category    CategoryFromString(std::string e);

    static std::string   FunctionCodeToString(FunctionCode e);
    static FunctionCode  FunctionCodeFromString(std::string e);

    using MoveDefPtr = std::shared_ptr<MoveDef>;

    MoveDef();

    friend void       tag_invoke(js::value_from_tag, js::value& jv, MoveDefPtr const& o);
    friend MoveDefPtr tag_invoke(js::value_to_tag<MoveDefPtr>, js::value const& jv);

    static MoveDefPtr              fromPropertyTree(std::string const& id, pt::ptree const& pt);
    static std::vector<MoveDefPtr> vectorFromPropertyTree(pt::ptree const& pt);

    std::string getId() const;
    void        setId(std::string const& newId);

    std::string getName() const;
    void        setName(std::string const& newName);

    std::string getType() const;
    void        setType(std::string const& newType);

    size_t getAccuracy() const;
    void   setAccuracy(size_t newAccuracy);

    size_t getPower() const;
    void   setPower(size_t newPower);

    size_t getTotalPP() const;
    void   setTotalPP(size_t newTotalPP);

    Category getCategory() const;
    void     setCategory(Category newCategory);

    int  getPriority() const;
    void setPriority(int newPriority);

    FunctionCode getFunctionCode() const;
    void         setFunctionCode(FunctionCode newFunctionCode);

    size_t getEffectChance() const;
    void   setEffectChance(size_t newEffectChance);

    Target getTarget() const;
    void   setTarget(Target newTarget);

    std::vector<Flag> const& getFlags() const;
    void                     setFlags(std::vector<Flag> const& newFlags);
    bool                     hasFlag(Flag flag) const;

    std::string getDescription() const;
    void        setDescription(std::string const& newDescription);

private:
    std::string  id;
    std::string  name;
    std::string  type;
    size_t       accuracy     = 0;
    size_t       power        = 0;
    size_t       totalPP      = 0;
    Category     category     = PHYSICAL;
    int          priority     = 0;
    FunctionCode      functionCode = FunctionCode::None;
    size_t            effectChance = 0;
    Target            target       = Target::NearOther;
    std::vector<Flag> flags;
    std::string       description;
};


#endif // MOVEDEF_H
