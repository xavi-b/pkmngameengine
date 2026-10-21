#include "moveeffects.h"

#include "abilitybattle.h"
#include "confusionstatus.h"
#include "game.h"
#include "itemutils.h"
#include "map.h"
#include "moveresolver.h"
#include "singlebattlescene.h"
#include "statuscondition.h"
#include "trainerscenedata.h"
#include "utils.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <cmath>

namespace lc = boost::locale;

StatChangeResult applyStatChange(SingleBattleSceneData& data, BattleBattler& battler, Pkmn::PkmnPtr const& pkmn,
                                 BattleBattler::StageStat stat, int delta, std::vector<std::string>& messages,
                                 bool fromOpponent, bool pierceAbilities, bool checkMist, bool announce, bool reflected)
{
    StatChangeResult result;
    if (!pkmn)
        return result;
    auto say = [&](std::string const& line) { messages.push_back(line); };
    if (fromOpponent && checkMist && delta < 0)
    {
        int mist = &battler == &data.playerBattler ? data.playerMist : data.opponentMist;
        if (mist > 0)
        {
            say((boost::format(lc::translate("%1% is protected by Mist!")) % pkmn->getDisplayName()).str());
            return result;
        }
    }
    if (fromOpponent && delta < 0 && !pierceAbilities && !reflected && !battler.abilitySuppressed
        && battler.abilityId == "MIRRORARMOR")
    {
        BattleBattler& other = &battler == &data.playerBattler ? data.opponentBattler : data.playerBattler;
        auto otherPkmn = &battler == &data.playerBattler ? data.opponentPkmn : data.playerPkmn;
        if (otherPkmn)
            say((boost::format(lc::translate("%1%'s Mirror Armor reflected the stat change!")) % otherPkmn->getDisplayName()).str());
        auto bounced = applyStatChange(data, other, otherPkmn, stat, delta, messages, true, false, checkMist, announce, true);
        result.changed = bounced.changed;
        result.applied = bounced.applied;
        result.raised  = bounced.raised;
        if (bounced.ejected)
        {
            if (&other == &data.playerBattler)
                data.playerEjectSwitch = true;
            else
                data.opponentEjectSwitch = true;
        }
        return result;
    }
    if (fromOpponent && delta < 0 && !pierceAbilities && !battler.abilitySuppressed && battler.abilityId == "FLOWERVEIL"
        && hasType(pkmn, "GRASS", &battler))
    {
        say((boost::format(lc::translate("%1%'s Flower Veil prevents stat loss!")) % pkmn->getDisplayName()).str());
        return result;
    }
    if (fromOpponent && delta < 0 && !battler.abilitySuppressed && battler.abilityId == "FULLMETALBODY")
    {
        say((boost::format(lc::translate("%1%'s %2% prevents stat loss!")) % pkmn->getDisplayName() % battler.abilityId).str());
        return result;
    }
    if (fromOpponent && delta < 0 && !pierceAbilities && !battler.abilitySuppressed
        && (battler.abilityId == "CLEARBODY" || battler.abilityId == "WHITESMOKE"
            || (battler.abilityId == "BIGPECKS" && stat == BattleBattler::StageStat::Defense)
            || (battler.abilityId == "HYPERCUTTER" && stat == BattleBattler::StageStat::Attack)
            || (battler.abilityId == "KEENEYE" && stat == BattleBattler::StageStat::Accuracy)))
    {
        say((boost::format(lc::translate("%1%'s %2% prevents stat loss!")) % pkmn->getDisplayName() % battler.abilityId).str());
        return result;
    }
    if (!battler.abilitySuppressed && battler.abilityId == "CONTRARY")
        delta = -delta;
    if (!battler.abilitySuppressed && battler.abilityId == "SIMPLE")
        delta *= 2;
    if (delta == 0 || !battler.changeStage(stat, delta))
    {
        say((boost::format(lc::translate(delta > 0 ? "%1%'s stats won't go any higher!" : "%1%'s stats won't go any lower!"))
             % pkmn->getDisplayName())
                .str());
        return result;
    }
    result.changed = true;
    result.applied = delta;
    if (delta > 0)
        result.raised = true;
    if (delta < 0)
    {
        battler.statsLoweredThisTurn = true;
        if (!battler.abilitySuppressed && battler.abilityId == "DEFIANT")
        {
            auto follow = applyStatChange(data, battler, pkmn, BattleBattler::StageStat::Attack, 2, messages, false, false,
                                          false, true);
            result.raised = result.raised || follow.raised;
            result.ejected = result.ejected || follow.ejected;
        }
        else if (!battler.abilitySuppressed && battler.abilityId == "COMPETITIVE")
        {
            auto follow = applyStatChange(data, battler, pkmn, BattleBattler::StageStat::SpecialAttack, 2, messages, false,
                                          false, false, true);
            result.raised = result.raised || follow.raised;
            result.ejected = result.ejected || follow.ejected;
        }
        if (ItemUtils::activeHeldId(pkmn, &battler) == "EJECTPACK")
        {
            ItemUtils::noteHeldItemLoss(pkmn, &battler, "EJECTPACK");
            pkmn->setHeldItem(nullptr);
            say((boost::format(lc::translate("%1% is switched out by its Eject Pack!")) % pkmn->getDisplayName()).str());
            result.ejected = true;
        }
    }
    if (announce)
    {
        char const* name = "stats";
        switch (stat)
        {
        case BattleBattler::StageStat::Attack: name = "Attack"; break;
        case BattleBattler::StageStat::Defense: name = "Defense"; break;
        case BattleBattler::StageStat::SpecialAttack: name = "Sp. Atk"; break;
        case BattleBattler::StageStat::SpecialDefense: name = "Sp. Def"; break;
        case BattleBattler::StageStat::Speed: name = "Speed"; break;
        case BattleBattler::StageStat::Accuracy: name = "accuracy"; break;
        case BattleBattler::StageStat::Evasion: name = "evasiveness"; break;
        default: break;
        }
        char const* how = "severely fell";
        if (delta >= 3)
            how = "rose drastically";
        else if (delta == 2)
            how = "sharply rose";
        else if (delta == 1)
            how = "rose";
        else if (delta == -1)
            how = "fell";
        else if (delta == -2)
            how = "harshly fell";
        say((boost::format(lc::translate("%1%'s %2% %3%!")) % pkmn->getDisplayName() % name % lc::translate(how)).str());
    }
    return result;
}

namespace
{
using SS = BattleBattler::StageStat;

int statAnimPalette(SS stat)
{
    switch (stat)
    {
    case SS::Attack:
        return 0;
    case SS::Defense:
        return 1;
    case SS::Accuracy:
        return 2;
    case SS::Speed:
        return 3;
    case SS::Evasion:
        return 4;
    case SS::SpecialAttack:
        return 5;
    case SS::SpecialDefense:
        return 6;
    default:
        return 7;
    }
}

void msg(MoveEffectContext& ctx, std::string const& text)
{
    ctx.messages.push_back(text);
}

void msgFmt(MoveEffectContext& ctx, boost::format const& f)
{
    ctx.messages.push_back(f.str());
}

void noteDirectDamage(MoveEffectContext& ctx, size_t amount)
{
    ctx.targetBattler.tookDamageThisTurn = true;
    ctx.targetBattler.lastDamage         = static_cast<int>(amount);
    if (ctx.def)
    {
        ctx.targetBattler.lastDamageCategory = ctx.def->getCategory();
        ctx.targetBattler.lastHitType = ctx.typeOverride.empty() ? ctx.def->getType() : ctx.typeOverride;
    }
}

// Fixed damage and OHKO share type, Substitute, Wonder Guard, Disguise, and Illusion.
// ohko skips Focus Sash and Endure. Sturdy vs OHKO is handled by the caller.
bool applyScriptedDamage(MoveEffectContext& ctx, size_t amount, bool ohko)
{
    if (!ctx.target || !ctx.def)
        return false;
    std::string attackType = ctx.typeOverride.empty() ? ctx.def->getType() : ctx.typeOverride;
    bool ignoreAbility = moveIgnoresTargetAbility(ctx.userBattler, ctx.def->getFunctionCode());
    bool pierceGround  = attackType == "GROUND" && grounded(ctx.target, ctx.targetBattler, ctx.data);
    DamageAdjustments adjust;
    adjust.pierceGroundImmunity = pierceGround;
    auto roll = SingleBattleScene::computeDamage(ctx.user, ctx.target, ctx.move, ctx.data.fieldWeather(), &ctx.userBattler,
                                                 &ctx.targetBattler, 1, true, false, 1.f, false,
                                                 ctx.typeOverride, adjust);
    if (roll.effectiveness <= 0.f)
    {
        msgFmt(ctx, boost::format(lc::translate("It doesn't affect %1%...")) % ctx.target->getDisplayName());
        ctx.immune = true;
        return false;
    }
    if (abilityGrantsImmunity(ctx.data, ctx.target, ctx.targetBattler, attackType, ignoreAbility, ctx.messages,
                              pierceGround))
        return false;
    if (!ignoreAbility && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "WONDERGUARD"
        && roll.effectiveness <= 1.f)
    {
        msgFmt(ctx, boost::format(lc::translate("%1%'s Wonder Guard protected it!")) % ctx.target->getDisplayName());
        return false;
    }
    if (ItemUtils::airBalloonBlocksGround(ctx.target, attackType, ctx.messages, &ctx.targetBattler))
        return false;

    bool bypassSub = ctx.def->getFunctionCode() == MoveDef::FunctionCode::RemoveProtectionsBypassSubstitute;
    if (ctx.targetBattler.substituteHP > 0 && !bypassSub)
    {
        ctx.hitSubstitute = true;
        int sub = ctx.targetBattler.substituteHP;
        int dmg = static_cast<int>(amount);
        if (dmg >= sub)
        {
            ctx.targetBattler.substituteHP = 0;
            msg(ctx, lc::translate("The substitute faded!"));
        }
        else
            ctx.targetBattler.substituteHP -= dmg;
        ctx.damageDealt = amount;
        return true;
    }

    bool brokeForm = false;
    if (!ignoreAbility && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "DISGUISE"
        && !ctx.targetBattler.disguiseBroken)
    {
        ctx.targetBattler.disguiseBroken = true;
        brokeForm = true;
        msgFmt(ctx, boost::format(lc::translate("%1%'s Disguise broke!")) % ctx.target->getDisplayName());
        amount = ctx.targetBattler.abilityId == "MAGICGUARD" ? 0 : std::max<size_t>(1, ctx.target->getTotalHP() / 8);
    }
    else if (!ignoreAbility && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "ICEFACE"
             && !ctx.targetBattler.iceFaceBroken && ctx.def->getCategory() == MoveDef::PHYSICAL)
    {
        ctx.targetBattler.iceFaceBroken = true;
        brokeForm = true;
        amount = 0;
        msgFmt(ctx, boost::format(lc::translate("%1%'s Ice Face took the hit!")) % ctx.target->getDisplayName());
    }
    else if (!ohko && ctx.target->getHP() == ctx.target->getTotalHP() && amount >= ctx.target->getHP()
             && ctx.target->getHP() > 1 && ItemUtils::activeHeldId(ctx.target, &ctx.targetBattler) == "FOCUSSASH")
    {
        amount = ctx.target->getHP() - 1;
        ItemUtils::noteHeldItemLoss(ctx.target, &ctx.targetBattler, "FOCUSSASH");
        ctx.target->setHeldItem(nullptr);
        msgFmt(ctx, boost::format(lc::translate("%1% hung on using its Focus Sash!")) % ctx.target->getDisplayName());
    }

    if (amount > 0)
        ctx.target->decreaseHP(amount);
    if (!ohko && ctx.targetBattler.endure && ctx.target->getHP() == 0)
    {
        ctx.target->setHP(1);
        ctx.targetBattler.endure = false;
        msgFmt(ctx, boost::format(lc::translate("%1% endured the hit!")) % ctx.target->getDisplayName());
    }
    if (amount > 0)
        noteDirectDamage(ctx, amount);
    if ((amount > 0 || brokeForm) && ctx.targetBattler.illusionActive)
    {
        ctx.targetBattler.illusionActive = false;
        ctx.targetBattler.illusionDisguise.reset();
        msgFmt(ctx, boost::format(lc::translate("%1%'s illusion wore off!")) % ctx.target->getDisplayName());
    }
    ctx.damageDealt = amount;
    ctx.moveHit     = true;
    return true;
}

bool ignoresTarget(MoveEffectContext const& ctx)
{
    return ctx.def && moveIgnoresTargetAbility(ctx.userBattler, ctx.def->getFunctionCode());
}

bool aromaVeil(MoveEffectContext const& ctx)
{
    if (ignoresTarget(ctx))
        return false;
    return !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "AROMAVEIL";
}

bool oblivious(MoveEffectContext const& ctx)
{
    if (ignoresTarget(ctx))
        return false;
    return !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "OBLIVIOUS";
}

bool stickyHold(MoveEffectContext const& ctx)
{
    if (ignoresTarget(ctx))
        return false;
    return !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "STICKYHOLD";
}

size_t flingPowerOf(Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler)
{
    if (ItemUtils::activeHeldId(pkmn, &battler).empty() || !pkmn || !pkmn->getHeldItem() || !pkmn->getHeldItem()->getDefinition())
        return 0;
    auto def = pkmn->getHeldItem()->getDefinition();
    using F = ItemDef::Flag;
    struct Row
    {
        F      flag;
        size_t power;
    };
    static Row const rows[] = {
        {F::Fling_250, 250}, {F::Fling_150, 150}, {F::Fling_130, 130}, {F::Fling_120, 120}, {F::Fling_110, 110},
        {F::Fling_100, 100}, {F::Fling_90, 90},   {F::Fling_85, 85},   {F::Fling_80, 80},   {F::Fling_75, 75},
        {F::Fling_70, 70},   {F::Fling_65, 65},   {F::Fling_60, 60},   {F::Fling_55, 55},   {F::Fling_50, 50},
        {F::Fling_40, 40},   {F::Fling_30, 30},   {F::Fling_25, 25},   {F::Fling_20, 20},   {F::Fling_10, 10},
    };
    for (auto const& row : rows)
    {
        if (def->hasFlag(row.flag))
            return row.power;
    }
    return 0;
}

bool berryOrGem(std::string const& id)
{
    if (id.find("BERRY") != std::string::npos)
        return true;
    return id.size() >= 3 && id.compare(id.size() - 3, 3, "GEM") == 0;
}

bool protectSucceeds(MoveEffectContext& ctx)
{
    int denom = 1;
    for (int i = 0; i < ctx.userBattler.protectStreak && denom <= 1000000000 / 3; ++i)
        denom *= 3;
    if (Utils::randuint(0, denom - 1) != 0)
    {
        ctx.userBattler.protectStreak = 0;
        msg(ctx, lc::translate("But it failed!"));
        return false;
    }
    if (ctx.userBattler.protectStreak < 6)
        ++ctx.userBattler.protectStreak;
    return true;
}

float battleWeight(Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler)
{
    float w = pkmn && pkmn->getDefinition() ? pkmn->getDefinition()->getWeight() : 0.1f;
    if (!battler.abilitySuppressed && battler.abilityId == "HEAVYMETAL")
        w *= 2.f;
    else if (!battler.abilitySuppressed && battler.abilityId == "LIGHTMETAL")
        w *= 0.5f;
    if (ItemUtils::activeHeldId(pkmn, &battler) == "FLOATSTONE")
        w *= 0.5f;
    w *= battler.weightFactor;
    return std::max(0.1f, w);
}

bool changeStage(MoveEffectContext& ctx, BattleBattler& battler, Pkmn::PkmnPtr const& pkmn, SS stat, int delta,
                 bool checkMist)
{
    if (&battler == &ctx.targetBattler && ctx.hitSubstitute)
        return false;
    bool fromOpponent = &battler == &ctx.targetBattler;
    // Damaging move secondaries on the foe need a real HP hit.
    if (fromOpponent && ctx.def && ctx.def->getCategory() != MoveDef::STATUS && ctx.damageDealt == 0)
        return false;
    auto outcome = applyStatChange(ctx.data, battler, pkmn, stat, delta, ctx.messages, fromOpponent,
                                   fromOpponent && ignoresTarget(ctx), checkMist, true);
    if (outcome.raised)
    {
        ctx.statRose = true;
        if (&battler == &ctx.targetBattler)
            ctx.statRoseOnTarget = true;
    }
    if (outcome.changed && outcome.applied < 0)
    {
        ctx.statFell = true;
        if (&battler == &ctx.targetBattler)
            ctx.statFellOnTarget = true;
    }
    if (outcome.changed && outcome.applied != 0)
    {
        StatAnimNote note;
        note.palette  = statAnimPalette(stat);
        note.rose     = outcome.applied > 0;
        note.onTarget = &battler == &ctx.targetBattler;
        note.sharp    = std::abs(outcome.applied) >= 2;
        ctx.statNotes.push_back(note);
    }
    if (outcome.ejected)
    {
        if (&battler == &ctx.userBattler)
            ctx.shouldSwitchUser = true;
        else
            ctx.shouldSwitchTarget = true;
    }
    return outcome.changed;
}

void spendStockpile(MoveEffectContext& ctx)
{
    if (ctx.userBattler.stockpileDef > 0)
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -ctx.userBattler.stockpileDef, false);
    if (ctx.userBattler.stockpileSpDef > 0)
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, -ctx.userBattler.stockpileSpDef, false);
    ctx.userBattler.stockpile      = 0;
    ctx.userBattler.stockpileDef   = 0;
    ctx.userBattler.stockpileSpDef = 0;
}

bool rollSecondary(MoveEffectContext& ctx)
{
    size_t chance = ctx.def->getEffectChance();
    if (chance > 0 && !ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "SHEERFORCE")
        return false;
    if (chance > 0 && ctx.target && !ignoresTarget(ctx) && !ctx.targetBattler.abilitySuppressed
        && ctx.targetBattler.abilityId == "SHIELDDUST")
        return false;
    if (chance == 0)
        chance = 100;
    if (!ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "SERENEGRACE")
        chance = std::min<size_t>(100, chance * 2);
    return Utils::randuint(1, 100) <= chance;
}

bool skipsRecoil(BattleBattler const& battler)
{
    return !battler.abilitySuppressed && (battler.abilityId == "ROCKHEAD" || battler.abilityId == "MAGICGUARD");
}

bool blocksConfusion(MoveEffectContext& ctx)
{
    if (!ctx.target || ctx.targetBattler.abilitySuppressed || ctx.targetBattler.abilityId != "OWNTEMPO" || ignoresTarget(ctx))
        return false;
    msgFmt(ctx, boost::format(lc::translate("%1%'s Own Tempo prevents confusion!")) % ctx.target->getDisplayName());
    return true;
}

void tryFlinch(MoveEffectContext& ctx)
{
    if (ctx.def && ctx.def->getCategory() != MoveDef::STATUS && ctx.damageDealt == 0)
        return;
    if (!ctx.target || ctx.hitSubstitute)
        return;
    if (!ignoresTarget(ctx) && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "INNERFOCUS")
    {
        msgFmt(ctx, boost::format(lc::translate("%1%'s Inner Focus kept it from flinching!")) % ctx.target->getDisplayName());
        return;
    }
    ctx.targetBattler.flinched = true;
    if (!ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "STEADFAST")
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, 1, false);
}

void healUser(MoveEffectContext& ctx, size_t amount)
{
    if (ctx.userBattler.healBlockTurns > 0)
        return;
    size_t maxHp = ctx.user->getTotalHP();
    size_t hp    = ctx.user->getHP();
    size_t newHp = std::min(maxHp, hp + amount);
    if (newHp == hp)
        return;
    ctx.user->setHP(newHp);
    ctx.healed = true;
    msgFmt(ctx, boost::format(lc::translate("%1% had its HP restored.")) % ctx.user->getDisplayName());
}

size_t maxHpOf(Pkmn::PkmnPtr const& p)
{
    return std::max<size_t>(1, p->getTotalHP());
}

bool tryStatus(MoveEffectContext& ctx, Pkmn::StatusCondition status)
{
    if (ctx.hitSubstitute)
        return false;
    // Damaging move secondaries need a real HP hit.
    if (ctx.def && ctx.def->getCategory() != MoveDef::STATUS && ctx.damageDealt == 0)
        return false;
    if (!ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "SHEERFORCE" && ctx.def
        && ctx.def->getEffectChance() > 0)
        return false;
    if (status == Pkmn::StatusCondition::SLEEP
        && (ctx.data.playerBattler.uproarTurns > 0 || ctx.data.opponentBattler.uproarTurns > 0))
    {
        msg(ctx, lc::translate("The uproar kept it awake!"));
        return false;
    }
    {
        AbilityEvent event;
        event.playerSide = !ctx.playerIsUser;
        event.status     = status;
        event.ignoreAbility = ignoresTarget(ctx);
        triggerAbility(ctx.data, AbilityMoment::OnStatus, event);
        if (event.blocked)
        {
            for (auto const& line : event.messages)
                msg(ctx, line);
            return false;
        }
    }
    int safeguard = ctx.playerIsUser ? ctx.data.opponentSafeguard : ctx.data.playerSafeguard;
    if (ctx.target && ctx.target != ctx.user && safeguard > 0)
    {
        msgFmt(ctx, boost::format(lc::translate("%1% is protected by Safeguard!")) % ctx.target->getDisplayName());
        return false;
    }
    bool corrosion = !ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "CORROSION";
    if (!applyStatusCondition(ctx.target, status, ctx.targetBattler.toxicCounter, corrosion, &ctx.targetBattler))
        return false;
    msg(ctx, statusInflictedMessage(ctx.target));
    if (!ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "SYNCHRONIZE" && ctx.user
        && ctx.user != ctx.target
        && (status == Pkmn::StatusCondition::BURN || status == Pkmn::StatusCondition::POISON
            || status == Pkmn::StatusCondition::PARALYSIS || status == Pkmn::StatusCondition::BADLY_POISON))
    {
        if (!abilityBlocksStatus(ctx.user, ctx.userBattler, status)
            && applyStatusCondition(ctx.user, status, ctx.userBattler.toxicCounter, false, &ctx.userBattler))
            msgFmt(ctx, boost::format(lc::translate("%1%'s Synchronize shared the status!")) % ctx.target->getDisplayName());
    }
    return true;
}

} // namespace

void noteMoveCommit(MoveEffectContext& ctx)
{
    if (!ctx.def || !ctx.user)
        return;
    auto code = ctx.def->getFunctionCode();
    if (code == MoveDef::FunctionCode::TypeAndPowerDependOnUserBerry)
    {
        std::string gift = ItemUtils::activeHeldId(ctx.user, &ctx.userBattler);
        auto itemDef = ctx.user->getHeldItem() ? ctx.user->getHeldItem()->getDefinition() : nullptr;
        if (!gift.empty() && itemDef && itemDef->getNaturalGiftPower() > 0)
        {
            ctx.userBattler.recycledItemId = itemDef->getId();
            ctx.userBattler.consumedBerry  = true;
            ItemUtils::noteHeldItemLoss(ctx.user, &ctx.userBattler, itemDef->getId());
            ctx.user->setHeldItem(nullptr);
        }
    }
    if (code == MoveDef::FunctionCode::PowerDependsOnUserStockpile && ctx.userBattler.stockpile > 0)
        spendStockpile(ctx);
    if (code == MoveDef::FunctionCode::MultiTurnAttackConfuseUserAtEnd && ctx.userBattler.multiTurnLock <= 0)
    {
        ctx.userBattler.multiTurnLock       = static_cast<int>(Utils::randuint(2, 3));
        ctx.userBattler.rampageMoveId       = ctx.def->getId();
        ctx.userBattler.rampageConfuseAtEnd = true;
    }
}

void rollHitFlinch(MoveEffectContext& ctx)
{
    if (rollSecondary(ctx))
        tryFlinch(ctx);
}

void armGulpMissile(MoveEffectContext& ctx)
{
    if (!ctx.user || ctx.damageDealt == 0 || !ctx.def)
        return;
    if (ctx.userBattler.abilitySuppressed || ctx.userBattler.abilityId != "GULPMISSILE")
        return;
    std::string id = ctx.def->getId();
    if (id != "SURF" && id != "DIVE")
        return;
    if (ctx.userBattler.gulpGulping || ctx.userBattler.gulpGorging)
        return;
    if (ctx.user->getHP() * 2 > ctx.user->getTotalHP())
        ctx.userBattler.gulpGulping = true;
    else
        ctx.userBattler.gulpGorging = true;
    refreshForme(ctx.data, ctx.playerIsUser);
    ctx.messages.push_back((boost::format(lc::translate("%1% gulped it down!")) % ctx.user->getDisplayName()).str());
}

void spitGulpMissile(MoveEffectContext& ctx)
{
    if (!ctx.target || (!ctx.targetBattler.gulpGulping && !ctx.targetBattler.gulpGorging))
        return;
    bool gorging = ctx.targetBattler.gulpGorging;
    ctx.targetBattler.gulpGulping = false;
    ctx.targetBattler.gulpGorging = false;
    refreshForme(ctx.data, !ctx.playerIsUser);
    bool magicGuard = ctx.user && !ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "MAGICGUARD";
    if (ctx.user && !magicGuard)
        ctx.user->decreaseHP(std::max<size_t>(1, ctx.target->getTotalHP() / 4));
    ctx.messages.push_back((boost::format(lc::translate("%1% spat out its catch!")) % ctx.target->getDisplayName()).str());
    if (gorging && ctx.user)
    {
        MoveEffectContext para(ctx.data, !ctx.playerIsUser, ctx.target, ctx.user, ctx.targetBattler, ctx.userBattler, ctx.move,
                               nullptr);
        tryStatus(para, Pkmn::StatusCondition::PARALYSIS);
        ctx.messages.insert(ctx.messages.end(), para.messages.begin(), para.messages.end());
    }
    else if (!gorging && ctx.user)
    {
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, true);
    }
}

int moveHitCount(MoveDef::MoveDefPtr const& def)
{
    if (!def)
        return 1;
    switch (def->getFunctionCode())
    {
    case MoveDef::FunctionCode::HitTwoTimes:
    case MoveDef::FunctionCode::HitTwoTimesFlinchTarget:
    case MoveDef::FunctionCode::HitTwoTimesPoisonTarget:
    case MoveDef::FunctionCode::HitTwoTimesTargetThenTargetAlly:
        return 2;
    case MoveDef::FunctionCode::HitThreeTimesAlwaysCriticalHit:
    case MoveDef::FunctionCode::HitThreeTimesPowersUpWithEachHit:
        return 3;
    case MoveDef::FunctionCode::HitTwoToFiveTimes:
    case MoveDef::FunctionCode::HitTwoToFiveTimesOrThreeForAshGreninja:
    case MoveDef::FunctionCode::HitTwoToFiveTimesRaiseUserSpd1LowerUserDef1:
    {
        // 2 hits 35%, 3 hits 35%, 4 hits 15%, 5 hits 15%.
        size_t r = Utils::randuint(0, 99);
        if (r < 35)
            return 2;
        if (r < 70)
            return 3;
        if (r < 85)
            return 4;
        return 5;
    }
    default:
        return 1;
    }
}

namespace
{
bool isHealingCode(MoveDef::FunctionCode code)
{
    switch (code)
    {
    case MoveDef::FunctionCode::CureTargetStatusHealUserHalfOfTotalHP:
    case MoveDef::FunctionCode::HealTargetDependingOnGrassyTerrain:
    case MoveDef::FunctionCode::HealTargetHalfOfTotalHP:
    case MoveDef::FunctionCode::HealUserAndAlliesQuarterOfTotalHP:
    case MoveDef::FunctionCode::HealUserAndAlliesQuarterOfTotalHPCureStatus:
    case MoveDef::FunctionCode::HealUserByHalfOfDamageDone:
    case MoveDef::FunctionCode::HealUserByHalfOfDamageDoneIfTargetAsleep:
    case MoveDef::FunctionCode::HealUserByTargetAttackLowerTargetAttack1:
    case MoveDef::FunctionCode::HealUserByThreeQuartersOfDamageDone:
    case MoveDef::FunctionCode::HealUserDependingOnSandstorm:
    case MoveDef::FunctionCode::HealUserDependingOnUserStockpile:
    case MoveDef::FunctionCode::HealUserDependingOnWeather:
    case MoveDef::FunctionCode::HealUserFullyAndFallAsleep:
    case MoveDef::FunctionCode::HealUserHalfOfTotalHP:
    case MoveDef::FunctionCode::HealUserHalfOfTotalHPLoseFlyingTypeThisTurn:
    case MoveDef::FunctionCode::HealUserPositionNextTurn:
    case MoveDef::FunctionCode::StartHealUserEachTurn:
    case MoveDef::FunctionCode::StartHealUserEachTurnTrapUserInBattle:
        return true;
    default:
        return false;
    }
}

bool hitsSemiInvulnerable(MoveDef::FunctionCode code, BattleBattler::SemiKind kind)
{
    switch (kind)
    {
    case BattleBattler::SemiKind::Sky:
        return code == MoveDef::FunctionCode::DoublePowerIfTargetInSky
            || code == MoveDef::FunctionCode::FlinchTargetDoublePowerIfTargetInSky
            || code == MoveDef::FunctionCode::ParalyzeTargetAlwaysHitsInRainHitsTargetInSky
            || code == MoveDef::FunctionCode::ConfuseTargetAlwaysHitsInRainHitsTargetInSky
            || code == MoveDef::FunctionCode::HitsTargetInSky
            || code == MoveDef::FunctionCode::HitsTargetInSkyGroundsTarget
            || code == MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSkyTargetCannotAct;
    case BattleBattler::SemiKind::Underground:
        return code == MoveDef::FunctionCode::DoublePowerIfTargetUnderground
            || code == MoveDef::FunctionCode::RandomPowerDoublePowerIfTargetUnderground
            || code == MoveDef::FunctionCode::OHKOHitsUndergroundTarget;
    case BattleBattler::SemiKind::Underwater:
        return code == MoveDef::FunctionCode::DoublePowerIfTargetUnderwater
            || code == MoveDef::FunctionCode::BindTargetDoublePowerIfTargetUnderwater;
    case BattleBattler::SemiKind::None:
        return false;
    }
    return false;
}

void setHide(BattleBattler& battler, BattleBattler::SemiKind kind)
{
    battler.semiKind = kind;
    battler.semiInvulnerable = kind != BattleBattler::SemiKind::None;
}

} // namespace

void releaseSkyDrop(SingleBattleSceneData& data, bool carrierIsPlayer)
{
    BattleBattler& target = carrierIsPlayer ? data.opponentBattler : data.playerBattler;
    if (!target.skyDropLocked)
        return;
    target.skyDropLocked = false;
    if (target.semiKind == BattleBattler::SemiKind::Sky)
    {
        target.semiKind         = BattleBattler::SemiKind::None;
        target.semiInvulnerable = false;
    }
}

void cancelTwoTurnCharge(SingleBattleSceneData& data, bool playerSide)
{
    BattleBattler& self = playerSide ? data.playerBattler : data.opponentBattler;
    if (self.skyDropLocked && !self.charging)
        return;
    if (!self.charging && self.semiKind == BattleBattler::SemiKind::None && !self.semiInvulnerable)
        return;
    self.charging         = false;
    self.semiKind         = BattleBattler::SemiKind::None;
    self.semiInvulnerable = false;
    releaseSkyDrop(data, playerSide);
}

void copyTransformFrom(Pkmn::PkmnPtr const& user,
                       BattleBattler&       userBattler,
                       Pkmn::PkmnPtr const& target,
                       BattleBattler const& targetBattler)
{
    if (!user || !target)
        return;
    userBattler.typesOverride = activeTypes(target, &targetBattler);
    userBattler.abilityId     = targetBattler.abilityId;
    userBattler.copyStagesFrom(targetBattler);
    auto copiedStat = [](int overrideValue, size_t base) {
        return overrideValue >= 0 ? overrideValue : static_cast<int>(base);
    };
    auto const& stats = target->getStats();
    userBattler.attackOverride    = copiedStat(targetBattler.attackOverride, stats.at(PkmnDef::ATTACK));
    userBattler.defenseOverride   = copiedStat(targetBattler.defenseOverride, stats.at(PkmnDef::DEFENSE));
    userBattler.spAttackOverride  = copiedStat(targetBattler.spAttackOverride, stats.at(PkmnDef::SPECIAL_ATTACK));
    userBattler.spDefenseOverride = copiedStat(targetBattler.spDefenseOverride, stats.at(PkmnDef::SPECIAL_DEFENSE));
    userBattler.speedOverride     = copiedStat(targetBattler.speedOverride, stats.at(PkmnDef::SPEED));
    auto& slots = user->getMoves();
    if (!userBattler.transformActive)
    {
        for (int i = 0; i < 4; ++i)
        {
            userBattler.transformMoveIds[i] = slots[i] && slots[i]->getDefinition() ? slots[i]->getDefinition()->getId() : "";
            userBattler.transformMovePP[i]  = slots[i] ? slots[i]->getCurrentPP() : 0;
        }
    }
    auto const& copied = target->getMoves();
    for (int i = 0; i < 4; ++i)
    {
        if (!copied[i] || !copied[i]->getDefinition())
        {
            slots[i].reset();
            continue;
        }
        auto move = std::make_shared<Move>(copied[i]->getDefinition());
        move->setCurrentPP(std::min<size_t>(5, copied[i]->getDefinition()->getTotalPP()));
        slots[i] = move;
    }
    userBattler.transformActive   = true;
    userBattler.transformDisguise = target;
    userBattler.usedMoveIds.clear();
}

bool grounded(Pkmn::PkmnPtr const& pkmn, BattleBattler const& battler, SingleBattleSceneData const& data)
{
    (void)data;
    if (!pkmn || battler.semiInvulnerable)
        return false;
    std::string item;
    bool klutz = !battler.abilitySuppressed && battler.abilityId == "KLUTZ";
    if (!battler.itemsBlocked && battler.embargoTurns <= 0 && !klutz && pkmn->getHeldItem()
        && pkmn->getHeldItem()->getDefinition())
        item = pkmn->getHeldItem()->getDefinition()->getId();
    if (item == "IRONBALL" || battler.groundedByMove)
        return true;
    if (battler.airborneTurns > 0)
        return false;
    if (item == "AIRBALLOON")
        return false;
    if (hasType(pkmn, "FLYING", &battler))
        return false;
    if (!battler.abilitySuppressed && battler.abilityId == "LEVITATE")
        return false;
    return true;
}

DelayedStrikeResult resolveDelayedStrike(SingleBattleSceneData& data, DelayedStrike& slot)
{
    DelayedStrikeResult result;
    result.hitsPlayer = slot.hitsPlayer;
    auto        attacker       = slot.user;
    bool        casterIsPlayer = slot.userIsPlayer;
    std::string moveId         = slot.moveId;
    size_t      storedDamage   = slot.damage;
    slot.turns                 = 0;
    slot.hitsPlayer            = false;
    slot.moveId.clear();
    slot.user.reset();
    slot.userIsPlayer = false;
    slot.damage       = 0;

    auto target = result.hitsPlayer ? data.playerPkmn : data.opponentPkmn;
    BattleBattler& targetBattler = result.hitsPlayer ? data.playerBattler : data.opponentBattler;
    if (!attacker || moveId.empty() || !target || target->isKO())
        return result;

    auto def = Game::instance()->data.moveDefFor(moveId);
    if (!def)
        return result;
    auto move = std::make_shared<Move>(def);

    BattleBattler  scratch;
    BattleBattler* attackerBattler = nullptr;
    auto           active          = casterIsPlayer ? data.playerPkmn : data.opponentPkmn;
    if (active == attacker)
        attackerBattler = casterIsPlayer ? &data.playerBattler : &data.opponentBattler;
    else
    {
        scratch.abilityId    = attacker->effectiveAbilityId();
        scratch.itemsBlocked = false;
        attackerBattler      = &scratch;
    }

    // Future Sight / Doom Desire bypass Protect-family; they still fail vs semi-invulnerability.
    if (targetBattler.semiInvulnerable)
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1% avoided the attack!")) % target->getDisplayName()).str());
        return result;
    }
    if (storedDamage > 0)
    {
        if (!targetBattler.abilitySuppressed && targetBattler.abilityId == "WONDERGUARD")
        {
            result.messages.push_back(
                (boost::format(lc::translate("%1%'s Wonder Guard protected it!")) % target->getDisplayName()).str());
            return result;
        }
        target->decreaseHP(storedDamage);
        result.messages.push_back(
            (boost::format(lc::translate("%1% was struck by the delayed move!")) % target->getDisplayName()).str());
        if (target->isKO())
        {
            result.fainted = true;
            result.messages.push_back(
                (boost::format(lc::translate("%1% fainted !")) % target->getDisplayName()).str());
        }
        return result;
    }

    MoveEffectContext ctx(data, casterIsPlayer, attacker, target, *attackerBattler, targetBattler, move, def);
    size_t            power      = moveEffectPower(ctx, def->getPower() > 0 ? def->getPower() : 80);
    std::string       attackType = ctx.typeOverride.empty() ? def->getType() : ctx.typeOverride;
    power = std::max<size_t>(1, static_cast<size_t>(static_cast<float>(power)
            * ItemUtils::heldTypePowerMultiplier(attacker, attackType, attackerBattler)));
    power = std::max<size_t>(1, static_cast<size_t>(static_cast<float>(power)
            * abilityTypePowerMultiplier(attacker, *attackerBattler, attackType)));

    float screen = 1.f;
    {
        int  reflect = result.hitsPlayer ? data.playerReflect : data.opponentReflect;
        int  light   = result.hitsPlayer ? data.playerLightScreen : data.opponentLightScreen;
        int  veil    = result.hitsPlayer ? data.playerAuroraVeil : data.opponentAuroraVeil;
        bool infiltrator = !attackerBattler->abilitySuppressed && attackerBattler->abilityId == "INFILTRATOR";
        auto category    = def->getCategory();
        if (!infiltrator
            && ((category == MoveDef::PHYSICAL && (reflect > 0 || veil > 0))
                || (category == MoveDef::SPECIAL && (light > 0 || veil > 0))))
            screen = 0.5f;
    }

    Map::Weather weather = data.fieldWeather();

    DamageAdjustments adjust;
    if (!attackerBattler->abilitySuppressed && attackerBattler->abilityId == "ADAPTABILITY")
        adjust.stabBonus = 2.f;
    if (!attackerBattler->abilitySuppressed && attackerBattler->abilityId == "UNAWARE")
        adjust.ignoreDefenseStages = true;
    if (!targetBattler.abilitySuppressed && targetBattler.abilityId == "UNAWARE")
        adjust.ignoreAttackStages = true;
    bool lucky = (result.hitsPlayer ? data.playerLuckyChant : data.opponentLuckyChant) > 0;
    if (lucky
        || (!targetBattler.abilitySuppressed
            && (targetBattler.abilityId == "BATTLEARMOR" || targetBattler.abilityId == "SHELLARMOR")))
        adjust.blockCrit = true;

    auto roll = SingleBattleScene::computeDamage(attacker, target, move, weather, attackerBattler, &targetBattler, power,
                                                 false, false, screen, false, attackType, adjust);
    if (roll.effectiveness <= 0.f)
    {
        result.messages.push_back(
            (boost::format(lc::translate("It doesn't affect %1%...")) % target->getDisplayName()).str());
        return result;
    }
    if (!targetBattler.abilitySuppressed && targetBattler.abilityId == "WONDERGUARD" && roll.effectiveness <= 1.f)
    {
        result.messages.push_back(
            (boost::format(lc::translate("%1%'s Wonder Guard protected it!")) % target->getDisplayName()).str());
        return result;
    }
    if (abilityGrantsImmunity(data, target, targetBattler, attackType, false, result.messages, false))
        return result;

    bool contact = def->hasFlag(MoveDef::Flag::Contact);
    roll.amount  = static_cast<size_t>(static_cast<float>(roll.amount)
            * abilityIncomingMultiplier(target, targetBattler, attackType, roll.effectiveness, roll.critical,
                                        def->getCategory(), contact, false, data.strongWinds));
    if (roll.amount == 0)
        return result;
    target->decreaseHP(roll.amount);
    result.messages.push_back(
        (boost::format(lc::translate("%1% was struck by the delayed move!")) % target->getDisplayName()).str());
    if (target->isKO())
    {
        result.fainted = true;
        result.messages.push_back(
            (boost::format(lc::translate("%1% fainted !")) % target->getDisplayName()).str());
    }
    return result;
}

namespace
{

bool callsAnotherMove(MoveDef::MoveDefPtr const& def)
{
    if (!def || def->getId() == "STRUGGLE")
        return true;
    switch (def->getFunctionCode())
    {
    case MoveDef::FunctionCode::UseLastMoveUsed:
    case MoveDef::FunctionCode::UseLastMoveUsedByTarget:
    case MoveDef::FunctionCode::UseRandomMove:
    case MoveDef::FunctionCode::UseRandomUserMoveIfAsleep:
    case MoveDef::FunctionCode::UseRandomMoveFromUserParty:
    case MoveDef::FunctionCode::UseMoveTargetIsAboutToUse:
    case MoveDef::FunctionCode::UseMoveDependingOnEnvironment:
    case MoveDef::FunctionCode::TargetUsesItsLastUsedMoveAgain:
        return true;
    default:
        return false;
    }
}

bool targetsFoeInSingles(MoveDef::Target t)
{
    return t == MoveDef::Target::NearFoe || t == MoveDef::Target::AllNearFoes || t == MoveDef::Target::FoeSide
        || t == MoveDef::Target::NearOther || t == MoveDef::Target::Other || t == MoveDef::Target::RandomNearFoe
        || t == MoveDef::Target::AllNearOthers || t == MoveDef::Target::AllBattlers;
}

bool isSketch(MoveDef::MoveDefPtr const& def)
{
    return def && def->getFunctionCode() == MoveDef::FunctionCode::ReplaceMoveWithTargetLastMoveUsed;
}

bool isMimic(MoveDef::MoveDefPtr const& def)
{
    return def && def->getFunctionCode() == MoveDef::FunctionCode::ReplaceMoveThisBattleWithTargetLastMoveUsed;
}

bool isBelch(MoveDef::MoveDefPtr const& def)
{
    return def && def->getFunctionCode() == MoveDef::FunctionCode::FailsIfUserNotConsumedBerry;
}

bool needsPriorTurnSetup(MoveDef::MoveDefPtr const& def)
{
    if (!def)
        return false;
    switch (def->getFunctionCode())
    {
    case MoveDef::FunctionCode::FailsIfUserDamagedThisTurn:
    case MoveDef::FunctionCode::UsedAfterUserTakesPhysicalDamage:
    case MoveDef::FunctionCode::BurnAttackerBeforeUserActs:
        return true;
    default:
        return false;
    }
}

bool isBideOrUproar(MoveDef::MoveDefPtr const& def)
{
    if (!def)
        return false;
    auto code = def->getFunctionCode();
    return code == MoveDef::FunctionCode::MultiTurnAttackBideThenReturnDoubleDamage
        || code == MoveDef::FunctionCode::MultiTurnAttackPreventSleeping;
}

bool isMultiTurnLocked(MoveDef::MoveDefPtr const& def)
{
    return def && (isTwoTurnChargingMove(def->getFunctionCode()) || isBideOrUproar(def));
}

bool isChatter(MoveDef::MoveDefPtr const& def)
{
    return def && def->getId() == "CHATTER";
}

bool bannedForInstruct(MoveDef::MoveDefPtr const& def)
{
    if (!def || callsAnotherMove(def) || isSketch(def) || isMimic(def) || isBelch(def) || needsPriorTurnSetup(def)
        || isMultiTurnLocked(def) || isChatter(def)
        || def->getFunctionCode() == MoveDef::FunctionCode::AttackAndSkipNextTurn)
        return true;
    switch (def->getFunctionCode())
    {
    case MoveDef::FunctionCode::ReplaceMoveThisBattleWithTargetLastMoveUsed:
    case MoveDef::FunctionCode::TransformUserIntoTarget:
    case MoveDef::FunctionCode::FailsIfUserDamagedThisTurn:
    case MoveDef::FunctionCode::UsedAfterUserTakesPhysicalDamage:
    case MoveDef::FunctionCode::BurnAttackerBeforeUserActs:
        return true;
    default:
        return false;
    }
}

bool bannedForAssist(MoveDef::MoveDefPtr const& def)
{
    if (!def || isChatter(def))
        return true;
    switch (def->getFunctionCode())
    {
    case MoveDef::FunctionCode::ProtectUser:
    case MoveDef::FunctionCode::ProtectUserBanefulBunker:
    case MoveDef::FunctionCode::ProtectUserFromDamagingMovesKingsShield:
    case MoveDef::FunctionCode::ProtectUserFromDamagingMovesObstruct:
    case MoveDef::FunctionCode::ProtectUserFromTargetingMovesSpikyShield:
    case MoveDef::FunctionCode::ProtectUserSideFromDamagingMovesIfUserFirstTurn:
    case MoveDef::FunctionCode::ProtectUserSideFromMultiTargetDamagingMoves:
    case MoveDef::FunctionCode::ProtectUserSideFromPriorityMoves:
    case MoveDef::FunctionCode::ProtectUserSideFromStatusMoves:
    case MoveDef::FunctionCode::UserEnduresFaintingThisTurn:
    case MoveDef::FunctionCode::CounterPhysicalDamage:
    case MoveDef::FunctionCode::CounterSpecialDamage:
    case MoveDef::FunctionCode::SwitchOutTargetDamagingMove:
    case MoveDef::FunctionCode::SwitchOutTargetStatusMove:
    case MoveDef::FunctionCode::UserTakesTargetItem:
    case MoveDef::FunctionCode::UserTargetSwapItems:
    case MoveDef::FunctionCode::TargetTakesUserItem:
    case MoveDef::FunctionCode::RedirectAllMovesToUser:
    case MoveDef::FunctionCode::RedirectAllMovesToTarget:
    case MoveDef::FunctionCode::PowerUpAllyMove:
    case MoveDef::FunctionCode::AttackerFaintsIfUserFaints:
    case MoveDef::FunctionCode::RemoveProtections:
    case MoveDef::FunctionCode::StealAndUseBeneficialStatusMove:
    case MoveDef::FunctionCode::TransformUserIntoTarget:
    case MoveDef::FunctionCode::TargetUsesItsLastUsedMoveAgain:
    case MoveDef::FunctionCode::TargetActsNext:
    case MoveDef::FunctionCode::TargetActsLast:
    case MoveDef::FunctionCode::DoesNothingCongratulations:
    case MoveDef::FunctionCode::DoesNothingFailsIfNoAlly:
    case MoveDef::FunctionCode::FlinchTargetFailsIfNotUserFirstTurn:
    case MoveDef::FunctionCode::CounterDamagePlusHalf:
    case MoveDef::FunctionCode::FailsIfUserDamagedThisTurn:
    case MoveDef::FunctionCode::UsedAfterUserTakesPhysicalDamage:
    case MoveDef::FunctionCode::BurnAttackerBeforeUserActs:
    case MoveDef::FunctionCode::ReplaceMoveWithTargetLastMoveUsed:
    case MoveDef::FunctionCode::ReplaceMoveThisBattleWithTargetLastMoveUsed:
        return true;
    default:
        return isTwoTurnChargingMove(def->getFunctionCode());
    }
}

bool sideHasReplacement(MoveEffectContext const& ctx, bool playerSide)
{
    auto otherAlive = [](auto const& party, Pkmn::PkmnPtr const& active) {
        for (auto const& member : party)
        {
            if (member && !member->isKO() && member != active)
                return true;
        }
        return false;
    };
    if (playerSide)
        return otherAlive(Game::instance()->data.player.pkmns, ctx.data.playerPkmn);
    if (auto* trainer = dynamic_cast<TrainerSceneData*>(&ctx.data))
    {
        if (trainer->opponentTrainer)
            return otherAlive(trainer->opponentTrainer->pkmns, ctx.data.opponentPkmn);
    }
    return false;
}

std::string plateType(std::string const& itemId)
{
    return ItemUtils::plateTypeForItem(itemId);
}

int hiddenPowerBits(Pkmn::PkmnPtr const& user, int mask)
{
    auto iv  = user->getIVs();
    auto bit = [&](PkmnDef::Stat stat) { return (iv[stat] & mask) ? 1 : 0; };
    return bit(PkmnDef::HP) + 2 * bit(PkmnDef::ATTACK) + 4 * bit(PkmnDef::DEFENSE) + 8 * bit(PkmnDef::SPEED)
         + 16 * bit(PkmnDef::SPECIAL_ATTACK) + 32 * bit(PkmnDef::SPECIAL_DEFENSE);
}

std::string hiddenPowerType(Pkmn::PkmnPtr const& user)
{
    static char const* types[] = {"FIGHTING", "FLYING", "POISON", "GROUND", "ROCK", "BUG", "GHOST", "STEEL",
                                  "FIRE", "WATER", "GRASS", "ELECTRIC", "PSYCHIC", "ICE", "DRAGON", "DARK"};
    int index = hiddenPowerBits(user, 1) * 15 / 63;
    return types[std::clamp(index, 0, 15)];
}

size_t hiddenPowerPower(Pkmn::PkmnPtr const& user)
{
    return static_cast<size_t>(hiddenPowerBits(user, 2) * 40 / 63 + 30);
}
} // namespace

int movePriority(MoveDef::MoveDefPtr const& def, Pkmn::PkmnPtr const& user, BattleBattler const& battler,
                 SingleBattleSceneData const& data)
{
    if (!def)
        return 0;
    int priority = def->getPriority();
    (void)data;
    if (!battler.abilitySuppressed && battler.abilityId == "PRANKSTER" && def->getCategory() == MoveDef::STATUS)
        ++priority;
    if (!battler.abilitySuppressed && battler.abilityId == "GALEWINGS" && def->getType() == "FLYING" && user
        && user->getHP() == user->getTotalHP())
        ++priority;
    if (!battler.abilitySuppressed && battler.abilityId == "TRIAGE" && isHealingCode(def->getFunctionCode()))
        priority += 3;
    return priority;
}

bool moveBlockedBeforeHit(MoveEffectContext& ctx)
{
    auto code = ctx.def->getFunctionCode();
    if (code == MoveDef::FunctionCode::ThrowUserItemAtTarget && flingPowerOf(ctx.user, ctx.userBattler) == 0)
    {
        msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    bool dampBlocks = code == MoveDef::FunctionCode::UserFaintsExplosive
                   || code == MoveDef::FunctionCode::UserFaintsPowersUpInMistyTerrainExplosive
                   || code == MoveDef::FunctionCode::UserLosesHalfOfTotalHPExplosive;
    auto dampHolder = [](BattleBattler const& battler) {
        return !battler.abilitySuppressed && battler.abilityId == "DAMP";
    };
    bool ignoreTarget = moveIgnoresTargetAbility(ctx.userBattler, code);
    if (dampBlocks && (dampHolder(ctx.userBattler) || (!ignoreTarget && dampHolder(ctx.targetBattler))))
    {
        msg(ctx, lc::translate("Damp prevents the explosion!"));
        return true;
    }
    if (!ignoreTarget && ctx.target && ctx.target != ctx.user && !ctx.targetBattler.abilitySuppressed)
    {
        if (ctx.targetBattler.abilityId == "BULLETPROOF" && ctx.def->hasFlag(MoveDef::Flag::Bomb))
        {
            msg(ctx, lc::translate("Bulletproof blocked the move!"));
            return true;
        }
        // Perish Song still affects other battlers; Soundproof is applied per-target there.
        if (ctx.targetBattler.abilityId == "SOUNDPROOF" && ctx.def->hasFlag(MoveDef::Flag::Sound)
            && code != MoveDef::FunctionCode::StartPerishCountsForAllBattlers)
        {
            msg(ctx, lc::translate("Soundproof blocked the move!"));
            return true;
        }
    }
    if (ctx.target && ctx.target != ctx.user && ctx.def->hasFlag(MoveDef::Flag::Powder))
    {
        bool goggles = ItemUtils::activeHeldId(ctx.target, &ctx.targetBattler) == "SAFETYGOGGLES";
        bool overcoat = !ignoreTarget && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "OVERCOAT";
        if (overcoat || goggles || hasType(ctx.target, "GRASS", &ctx.targetBattler))
        {
            msg(ctx, lc::translate("The powder had no effect!"));
            return true;
        }
    }
    if (ctx.target && ctx.target != ctx.user && ctx.targetBattler.semiInvulnerable && !ctx.userBattler.lockOn
        && !hitsSemiInvulnerable(code, ctx.targetBattler.semiKind))
    {
        msg(ctx, lc::translate("The attack missed!"));
        return true;
    }
    int priority = movePriority(ctx.def, ctx.user, ctx.userBattler, ctx.data);
    if (!ignoreTarget && ctx.target && ctx.target != ctx.user && priority > 0 && !ctx.targetBattler.abilitySuppressed
        && (ctx.targetBattler.abilityId == "QUEENLYMAJESTY" || ctx.targetBattler.abilityId == "DAZZLING"))
    {
        msg(ctx, lc::translate("The priority move was blocked!"));
        return true;
    }
    if (ctx.target && ctx.target != ctx.user && ctx.def->getCategory() == MoveDef::STATUS
        && !ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "PRANKSTER"
        && hasType(ctx.target, "DARK", &ctx.targetBattler))
    {
        msg(ctx, lc::translate("It doesn't affect the Dark-type Pokémon..."));
        return true;
    }
    if (code == MoveDef::FunctionCode::HealUserByHalfOfDamageDoneIfTargetAsleep
        && (!ctx.target || ctx.target->getStatusCondition() != Pkmn::StatusCondition::SLEEP))
    {
        msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    if (code == MoveDef::FunctionCode::UserLosesFireType && !hasType(ctx.user, "FIRE", &ctx.userBattler))
    {
        msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    if (code == MoveDef::FunctionCode::PowerDependsOnUserStockpile && ctx.userBattler.stockpile <= 0)
    {
        msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    if (code == MoveDef::FunctionCode::FailsIfUserDamagedThisTurn && ctx.userBattler.tookDamageThisTurn)
    {
        msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    if (code == MoveDef::FunctionCode::UsedAfterUserTakesPhysicalDamage
        && (!ctx.userBattler.tookDamageThisTurn || ctx.userBattler.lastDamageCategory != MoveDef::PHYSICAL))
    {
        msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    if (code == MoveDef::FunctionCode::FailsIfTargetActed)
    {
        Move::MovePtr chosen = ctx.playerIsUser ? ctx.data.encounterMove : ctx.data.playerMove;
        bool choseMove = ctx.playerIsUser ? (ctx.data.opponentAction == BattleActions::Type::MOVES)
                                          : (ctx.data.playerAction == BattleActions::Type::MOVES);
        bool damaging = choseMove && chosen && chosen->getDefinition()
                     && chosen->getDefinition()->getCategory() != MoveDef::STATUS;
        if (ctx.targetBattler.movedThisTurn || ctx.targetBattler.switchedInThisTurn || !damaging)
        {
            msg(ctx, lc::translate("But it failed!"));
            return true;
        }
    }
    if (code == MoveDef::FunctionCode::FailsIfUserHasUnusedMove)
    {
        for (auto const& slot : ctx.user->getMoves())
        {
            if (!slot || !slot->getDefinition() || slot->getDefinition()->getId() == ctx.def->getId())
                continue;
            auto const& used = ctx.userBattler.usedMoveIds;
            if (std::find(used.begin(), used.end(), slot->getDefinition()->getId()) == used.end())
            {
                msg(ctx, lc::translate("But it failed!"));
                return true;
            }
        }
    }
    if (code == MoveDef::FunctionCode::FailsUnlessTargetSharesTypeWithUser && ctx.target)
    {
        bool shared = false;
        for (auto const& type : activeTypes(ctx.user, &ctx.userBattler))
        {
            if (hasType(ctx.target, type, &ctx.targetBattler))
                shared = true;
        }
        if (!shared)
        {
            msg(ctx, lc::translate("But it failed!"));
            return true;
        }
    }
    if (code == MoveDef::FunctionCode::FailsIfTargetHasNoItem && ctx.target && !ctx.target->getHeldItem())
    {
        msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    if (code == MoveDef::FunctionCode::FailsIfUserNotConsumedBerry && !ctx.userBattler.consumedBerry)
    {
        msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    if (code == MoveDef::FunctionCode::TypeAndPowerDependOnUserBerry)
    {
        auto itemDef = ctx.user && ctx.user->getHeldItem() ? ctx.user->getHeldItem()->getDefinition() : nullptr;
        if (!itemDef || itemDef->getNaturalGiftPower() == 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return true;
        }
    }
    if (code == MoveDef::FunctionCode::HoopaRemoveProtectionsBypassSubstituteLowerUserDef1)
    {
        std::string sid = ctx.user && ctx.user->getDefinition() ? ctx.user->getDefinition()->getId() : std::string();
        bool unbound = sid == "HOOPAUNBOUND"
                    || (sid == "HOOPA" && !ctx.userBattler.spriteFormeSuffix.empty());
        if (!unbound)
        {
            msg(ctx, lc::translate("But it failed!"));
            return true;
        }
    }
    if (ctx.userBattler.healBlockTurns > 0 && isHealingCode(code))
    {
        msgFmt(ctx, boost::format(lc::translate("%1% can't use healing moves!")) % ctx.user->getDisplayName());
        return true;
    }
    if (ctx.userBattler.soundBlockTurns > 0 && ctx.def->hasFlag(MoveDef::Flag::Sound))
    {
        msgFmt(ctx, boost::format(lc::translate("%1% can't use sound moves!")) % ctx.user->getDisplayName());
        return true;
    }
    if (ctx.userBattler.torment && ctx.userBattler.lastMoveId == ctx.def->getId())
    {
        msgFmt(ctx, boost::format(lc::translate("%1% can't use the same move twice!")) % ctx.user->getDisplayName());
        return true;
    }
    if (ctx.userBattler.tauntTurns > 0 && ctx.def->getCategory() == MoveDef::STATUS)
    {
        msgFmt(ctx, boost::format(lc::translate("%1% can't use status moves!")) % ctx.user->getDisplayName());
        return true;
    }
    if (ctx.targetBattler.imprison && ctx.target)
    {
        for (auto const& slot : ctx.target->getMoves())
        {
            if (slot && slot->getDefinition() && slot->getDefinition()->getId() == ctx.def->getId())
            {
                msg(ctx, lc::translate("Imprison blocks that move!"));
                return true;
            }
        }
    }
    auto blocksWeather = [&](char const* abilityId, std::string const& type) {
        auto check = [&](BattleBattler const& battler) {
            return !battler.abilitySuppressed && battler.abilityId == abilityId;
        };
        return ctx.def->getType() == type && (check(ctx.userBattler) || check(ctx.targetBattler));
    };
    if (blocksWeather("PRIMORDIALSEA", "FIRE") || blocksWeather("DESOLATELAND", "WATER"))
    {
        msg(ctx, lc::translate("The move evaporated in the extreme weather!"));
        return true;
    }
    bool userGoggles = ItemUtils::activeHeldId(ctx.user, &ctx.userBattler) == "SAFETYGOGGLES";
    if (ctx.userBattler.powder && ctx.def->getType() == "FIRE" && !userGoggles)
    {
        bool magicGuard = !ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "MAGICGUARD";
        if (!magicGuard)
        {
            ctx.user->decreaseHP(std::max<size_t>(1, ctx.user->getTotalHP() / 4));
            msgFmt(ctx, boost::format(lc::translate("%1% was hurt by the powder!")) % ctx.user->getDisplayName());
        }
        ctx.userBattler.powder = false;
        return true;
    }
    return false;
}

bool isTwoTurnChargingMove(MoveDef::FunctionCode code)
{
    switch (code)
    {
    case MoveDef::FunctionCode::TwoTurnAttack:
    case MoveDef::FunctionCode::TwoTurnAttackBurnTarget:
    case MoveDef::FunctionCode::TwoTurnAttackChargeRaiseUserDefense1:
    case MoveDef::FunctionCode::TwoTurnAttackChargeRaiseUserSpAtk1:
    case MoveDef::FunctionCode::TwoTurnAttackFlinchTarget:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSky:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSkyParalyzeTarget:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSkyTargetCannotAct:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableRemoveProtections:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableUnderground:
    case MoveDef::FunctionCode::TwoTurnAttackInvulnerableUnderwater:
    case MoveDef::FunctionCode::TwoTurnAttackOneTurnInSun:
    case MoveDef::FunctionCode::TwoTurnAttackParalyzeTarget:
    case MoveDef::FunctionCode::TwoTurnAttackRaiseUserSpAtkSpDefSpd2:
        return true;
    default:
        return false;
    }
}

namespace
{
void eatHeldBerry(MoveEffectContext& ctx, Pkmn::PkmnPtr const& pkmn, BattleBattler& battler, ConfusionStatus* confusion)
{
    if (!pkmn || pkmn->isKO() || !pkmn->getHeldItem() || !pkmn->getHeldItem()->getDefinition())
        return;
    std::string itemId = pkmn->getHeldItem()->getDefinition()->getId();
    if (itemId.find("BERRY") == std::string::npos)
        return;
    auto berry = pkmn->getHeldItem();
    pkmn->setHeldItem(nullptr);
    auto eaten = ItemUtils::eatBerry(pkmn, &battler, confusion, berry, ctx.data);
    battler.recycledItemId = itemId;
    battler.consumedItemId = itemId;
    battler.consumedBerry  = true;
    if (!battler.abilitySuppressed && battler.abilityId == "UNBURDEN")
        battler.unburdenActive = true;
    if (eaten.ejected)
    {
        if (&battler == &ctx.userBattler)
            ctx.shouldSwitchUser = true;
        else
            ctx.shouldSwitchTarget = true;
    }
    if (!eaten.message.empty())
        msg(ctx, eaten.message);
}

bool callMove(MoveEffectContext& ctx, MoveDef::MoveDefPtr const& calledDef, bool instantTwoTurn = false)
{
    if (!calledDef || ctx.callDepth > 0)
    {
        msg(ctx, lc::translate("But it failed!"));
        return false;
    }
    auto called = std::make_shared<Move>(calledDef);
    auto result = MoveResolver::resolve(ctx.data, ctx.playerIsUser, called, ctx.callDepth + 1, instantTwoTurn);
    ctx.messages.insert(ctx.messages.end(), result.messages.begin(), result.messages.end());
    ctx.shouldSwitchUser   = result.shouldSwitchUser;
    ctx.shouldSwitchTarget = result.shouldSwitchTarget;
    ctx.forcedSwitchTarget = result.forcedSwitchTarget;
    ctx.forcedSwitchUser   = result.forcedSwitchUser;
    return true;
}

MoveDef::MoveDefPtr moveById(std::string const& id)
{
    return Game::instance()->data.moveDefFor(id);
}
} // namespace

bool isRechargeMove(MoveDef::FunctionCode code)
{
    return code == MoveDef::FunctionCode::AttackAndSkipNextTurn;
}

bool gravityBlocksMove(MoveDef::FunctionCode code)
{
    return code == MoveDef::FunctionCode::CrashDamageIfFailsUnusableInGravity
        || code == MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSky
        || code == MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSkyParalyzeTarget
        || code == MoveDef::FunctionCode::TwoTurnAttackInvulnerableInSkyTargetCannotAct
        || code == MoveDef::FunctionCode::TwoTurnAttackInvulnerableUnderground
        || code == MoveDef::FunctionCode::TwoTurnAttackInvulnerableUnderwater
        || code == MoveDef::FunctionCode::DoesNothingUnusableInGravity
        || code == MoveDef::FunctionCode::StartUserAirborne
        || code == MoveDef::FunctionCode::StartTargetAirborneAndAlwaysHitByMoves;
}

void armSelectSetupMove(BattleBattler& battler, Move::MovePtr const& move)
{
    if (!move || !move->getDefinition())
        return;
    auto code = move->getDefinition()->getFunctionCode();
    if (code == MoveDef::FunctionCode::BurnAttackerBeforeUserActs)
        battler.beakBlast = true;
    if (code == MoveDef::FunctionCode::UsedAfterUserTakesPhysicalDamage)
        battler.shellTrap = true;
}

void appendSelectAnnounce(std::vector<std::string>& messages, Pkmn::PkmnPtr const& pkmn, Move::MovePtr const& move)
{
    if (!pkmn || !move || !move->getDefinition())
        return;
    auto code = move->getDefinition()->getFunctionCode();
    if (code == MoveDef::FunctionCode::FailsIfUserDamagedThisTurn)
        messages.push_back(
            (boost::format(lc::translate("%1% is tightening its focus!")) % pkmn->getDisplayName()).str());
    else if (code == MoveDef::FunctionCode::BurnAttackerBeforeUserActs)
        messages.push_back(
            (boost::format(lc::translate("%1% started heating its beak!")) % pkmn->getDisplayName()).str());
    else if (code == MoveDef::FunctionCode::UsedAfterUserTakesPhysicalDamage)
        messages.push_back(
            (boost::format(lc::translate("%1% set a shell trap!")) % pkmn->getDisplayName()).str());
}

void clearEncoreIfMoveDepleted(Pkmn::PkmnPtr const& pkmn, BattleBattler& battler)
{
    if (!pkmn || battler.encoreTurns <= 0 || battler.encoreMoveId.empty())
        return;
    for (auto const& slot : pkmn->getMoves())
    {
        if (slot && slot->getDefinition() && slot->getDefinition()->getId() == battler.encoreMoveId
            && slot->getCurrentPP() > 0)
            return;
    }
    battler.encoreTurns = 0;
    battler.encoreMoveId.clear();
}

size_t moveEffectPower(MoveEffectContext& ctx, size_t basePower)
{
    auto code = ctx.def->getFunctionCode();
    size_t power = basePower;

    auto positiveStages = [](BattleBattler const& b) {
        int n = 0;
        for (size_t i = 0; i < BattleBattler::StageStatCount; ++i)
            if (b.stages[i] > 0)
                n += b.stages[i];
        return n;
    };

    switch (code)
    {
    case MoveDef::FunctionCode::UserFaintsPowersUpInMistyTerrainExplosive:
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetActed:
        if (ctx.targetBattler.movedThisTurn)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetNotActed:
        if (!ctx.targetBattler.movedThisTurn)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetHPLessThanHalf:
        if (ctx.target && ctx.target->getHP() * 2 <= ctx.target->getTotalHP())
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetStatusProblem:
        if (ctx.target && ctx.target->getStatusCondition() != Pkmn::StatusCondition::NONE)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetPoisoned:
        if (ctx.target
            && (ctx.target->getStatusCondition() == Pkmn::StatusCondition::POISON
                || ctx.target->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON))
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetAsleepCureTarget:
        if (ctx.target && ctx.target->getStatusCondition() == Pkmn::StatusCondition::SLEEP)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetParalyzedCureTarget:
        if (ctx.target && ctx.target->getStatusCondition() == Pkmn::StatusCondition::PARALYSIS)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfUserPoisonedBurnedParalyzed:
        if (ctx.user->getStatusCondition() == Pkmn::StatusCondition::POISON
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::BURN
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::PARALYSIS)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfUserHasNoItem:
        if (!ctx.user->getHeldItem())
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetUnderground:
    case MoveDef::FunctionCode::DoublePowerIfTargetUnderwater:
    case MoveDef::FunctionCode::DoublePowerIfTargetInSky:
    case MoveDef::FunctionCode::FlinchTargetDoublePowerIfTargetInSky:
    case MoveDef::FunctionCode::BindTargetDoublePowerIfTargetUnderwater:
        if (hitsSemiInvulnerable(code, ctx.targetBattler.semiKind))
            power *= 2;
        break;
    case MoveDef::FunctionCode::RandomPowerDoublePowerIfTargetUnderground:
    {
        int const powers[]  = {10, 30, 50, 70, 90, 110, 150};
        int const weights[] = {5, 10, 20, 30, 20, 10, 5};
        int       roll      = static_cast<int>(Utils::randuint(1, 100));
        int       tally     = 0;
        power               = powers[0];
        for (size_t i = 0; i < 7; ++i)
        {
            tally += weights[i];
            if (roll <= tally)
            {
                power = powers[i];
                break;
            }
        }
        if (hitsSemiInvulnerable(code, ctx.targetBattler.semiKind))
            power *= 2;
        break;
    }
    case MoveDef::FunctionCode::DoublePowerInElectricTerrain:
        break;
    case MoveDef::FunctionCode::DoublePowerAfterFusionBolt:
        if (ctx.data.fusionMoveId == "FUSIONBOLT")
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerAfterFusionFlare:
        if (ctx.data.fusionMoveId == "FUSIONFLARE")
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfAllyFaintedLastTurn:
        if ((ctx.playerIsUser && ctx.data.playerRetaliate) || (!ctx.playerIsUser && ctx.data.opponentRetaliate))
            power *= 2;
        break;
    case MoveDef::FunctionCode::PursueSwitchingFoe:
        if (ctx.data.pursuitStrike)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfTargetLostHPThisTurn:
        if (ctx.targetBattler.tookDamageThisTurn)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfUserLastMoveFailed:
        if (ctx.userBattler.lastMoveFailed)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfUserLostHPThisTurn:
        if (ctx.userBattler.tookDamageThisTurn)
            power *= 2;
        break;
    case MoveDef::FunctionCode::DoublePowerIfUserStatsLoweredThisTurn:
        if (ctx.userBattler.statsLoweredThisTurn)
            power *= 2;
        break;
    case MoveDef::FunctionCode::PowerHigherWithUserHP:
        power = std::max<size_t>(1, basePower * ctx.user->getHP() / maxHpOf(ctx.user));
        break;
    case MoveDef::FunctionCode::PowerLowerWithUserHP: {
        size_t maxHp = std::max<size_t>(1, maxHpOf(ctx.user));
        size_t ratio = ctx.user->getHP() * 48 / maxHp;
        if (ratio < 2)
            power = 200;
        else if (ratio < 5)
            power = 150;
        else if (ratio < 10)
            power = 100;
        else if (ratio < 17)
            power = 80;
        else if (ratio < 33)
            power = 40;
        else
            power = 20;
        break;
    }
    case MoveDef::FunctionCode::PowerHigherWithTargetHP:
        if (ctx.target)
        {
            size_t maxHp = std::max<size_t>(1, maxHpOf(ctx.target));
            power = 1 + 120 * ctx.target->getHP() / maxHp;
        }
        break;
    case MoveDef::FunctionCode::TypeDependsOnUserMorpekoFormRaiseUserSpeed1:
        if (ctx.userBattler.hangry)
            ctx.typeOverride = "DARK";
        break;
    case MoveDef::FunctionCode::PowerHigherWithUserPositiveStatStages:
        power = std::max<size_t>(1, 20 * (1 + positiveStages(ctx.userBattler)));
        break;
    case MoveDef::FunctionCode::PowerHigherWithTargetPositiveStatStages:
        power = std::max<size_t>(1, 20 * (1 + positiveStages(ctx.targetBattler)));
        break;
    case MoveDef::FunctionCode::PowerDependsOnUserStockpile:
        power = static_cast<size_t>(100 * std::max(0, ctx.userBattler.stockpile));
        break;
    case MoveDef::FunctionCode::PowerHigherWithLessPP:
    {
        size_t pp = ctx.move ? ctx.move->getCurrentPP() : 0;
        if (pp >= 4)
            power = 40;
        else if (pp == 3)
            power = 50;
        else if (pp == 2)
            power = 60;
        else if (pp == 1)
            power = 80;
        else
            power = 200;
        break;
    }
    case MoveDef::FunctionCode::PowerHigherWithUserFasterThanTarget:
        if (ctx.target)
        {
            size_t theirs = std::max<size_t>(1, battleSpeed(ctx.target, &ctx.targetBattler, ctx.data.fieldWeather(), false));
            float  ratio  = static_cast<float>(battleSpeed(ctx.user, &ctx.userBattler, ctx.data.fieldWeather(), false))
                          / static_cast<float>(theirs);
            if (ratio >= 4.f)
                power = 150;
            else if (ratio >= 3.f)
                power = 120;
            else if (ratio >= 2.f)
                power = 80;
            else if (ratio >= 1.f)
                power = 60;
            else
                power = 40;
        }
        break;
    case MoveDef::FunctionCode::PowerHigherWithTargetFasterThanUser:
        if (ctx.target)
        {
            size_t mine   = std::max<size_t>(1, battleSpeed(ctx.user, &ctx.userBattler, ctx.data.fieldWeather(), false));
            size_t theirs = battleSpeed(ctx.target, &ctx.targetBattler, ctx.data.fieldWeather(), false);
            power         = std::min<size_t>(150, 25 * theirs / mine + 1);
        }
        break;
    case MoveDef::FunctionCode::PowerHigherWithUserHappiness:
        power = std::max<size_t>(1, ctx.user->getHappiness() * 2 / 5);
        break;
    case MoveDef::FunctionCode::PowerLowerWithUserHappiness:
        power = std::max<size_t>(1, (255 - ctx.user->getHappiness()) * 2 / 5);
        break;
    case MoveDef::FunctionCode::PowerHigherWithUserHeavierThanTarget:
        if (ctx.user && ctx.target && ctx.user->getDefinition() && ctx.target->getDefinition())
        {
            float userW   = battleWeight(ctx.user, ctx.userBattler);
            float targetW = battleWeight(ctx.target, ctx.targetBattler);
            float ratio   = userW / targetW;
            if (ratio >= 5.f)
                power = 120;
            else if (ratio >= 4.f)
                power = 100;
            else if (ratio >= 3.f)
                power = 80;
            else if (ratio >= 2.f)
                power = 60;
            else
                power = 40;
        }
        break;
    case MoveDef::FunctionCode::PowerHigherWithTargetWeight:
        if (ctx.target && ctx.target->getDefinition())
        {
            float w = battleWeight(ctx.target, ctx.targetBattler);
            if (w < 10.f)
                power = 20;
            else if (w < 25.f)
                power = 40;
            else if (w < 50.f)
                power = 60;
            else if (w < 100.f)
                power = 80;
            else if (w < 200.f)
                power = 100;
            else
                power = 120;
        }
        break;
    case MoveDef::FunctionCode::LowerTargetDefense1PowersUpInGravity:
    case MoveDef::FunctionCode::LowerTargetSpeed1WeakerInGrassyTerrain:
        break;
    case MoveDef::FunctionCode::AlwaysCriticalHit:
        break;
    case MoveDef::FunctionCode::TypeAndPowerDependOnWeather: {
        power = 50;
        ctx.typeOverride = "NORMAL";
        bool umbrella = ItemUtils::activeHeldId(ctx.user, &ctx.userBattler) == "UTILITYUMBRELLA";
        if (!umbrella && ctx.data.fieldWeather() == Map::Weather::HARSH_SUNLIGHT)
        {
            power = 100;
            ctx.typeOverride = "FIRE";
        }
        else if (!umbrella && ctx.data.fieldWeather() == Map::Weather::RAIN)
        {
            power = 100;
            ctx.typeOverride = "WATER";
        }
        else if (ctx.data.fieldWeather() == Map::Weather::SANDSTORM)
        {
            power = 100;
            ctx.typeOverride = "ROCK";
        }
        else if (ctx.data.fieldWeather() == Map::Weather::HAIL)
        {
            power = 100;
            ctx.typeOverride = "ICE";
        }
        break;
    }
    case MoveDef::FunctionCode::TypeAndPowerDependOnTerrain:
        power = basePower > 0 ? basePower : 50;
        break;
    case MoveDef::FunctionCode::HitsAllFoesAndPowersUpInPsychicTerrain:
        break;
    case MoveDef::FunctionCode::TypeIsUserFirstType: {
        auto types = activeTypes(ctx.user, &ctx.userBattler);
        if (!types.empty())
            ctx.typeOverride = types.front();
        break;
    }
    case MoveDef::FunctionCode::TypeAndPowerDependOnUserBerry: {
        std::string gift = ItemUtils::activeHeldId(ctx.user, &ctx.userBattler);
        auto itemDef = ctx.user && ctx.user->getHeldItem() ? ctx.user->getHeldItem()->getDefinition() : nullptr;
        if (!gift.empty() && itemDef && itemDef->getNaturalGiftPower() > 0)
        {
            auto item = itemDef;
            power = item->getNaturalGiftPower();
            ctx.typeOverride = item->getNaturalGiftType();
        }
        break;
    }
    case MoveDef::FunctionCode::TypeDependsOnUserPlate:
    case MoveDef::FunctionCode::TypeDependsOnUserDrive:
    case MoveDef::FunctionCode::TypeDependsOnUserMemory: {
        std::string plate = ItemUtils::activeHeldId(ctx.user, &ctx.userBattler);
        if (!plate.empty())
        {
            auto typed = plateType(plate);
            if (!typed.empty())
                ctx.typeOverride = typed;
        }
        break;
    }
    case MoveDef::FunctionCode::TypeDependsOnUserIVs:
        power = hiddenPowerPower(ctx.user);
        ctx.typeOverride = hiddenPowerType(ctx.user);
        break;
    case MoveDef::FunctionCode::PowerHigherWithConsecutiveUse:
        if (ctx.userBattler.furyCutterMoveId == ctx.def->getId() && ctx.userBattler.furyCutterPower > 0)
            power = std::min<size_t>(160, static_cast<size_t>(ctx.userBattler.furyCutterPower));
        break;
    case MoveDef::FunctionCode::PowerHigherWithConsecutiveUseOnUserSide: {
        int echo = ctx.playerIsUser ? ctx.data.playerEchoedVoice : ctx.data.opponentEchoedVoice;
        if (echo > 0)
            power = static_cast<size_t>(echo);
        break;
    }
    case MoveDef::FunctionCode::MultiTurnAttackPowersUpEachTurn: {
        size_t chain = basePower;
        if (ctx.userBattler.rolloutMoveId == ctx.def->getId() && ctx.userBattler.rolloutPower > 0)
            chain = std::min<size_t>(480, static_cast<size_t>(ctx.userBattler.rolloutPower) * 2);
        ctx.userBattler.rolloutPower = static_cast<int>(chain);
        power = ctx.userBattler.defenseCurl ? std::min<size_t>(480, chain * 2) : chain;
        break;
    }
    case MoveDef::FunctionCode::ThrowUserItemAtTarget:
        power = flingPowerOf(ctx.user, ctx.userBattler);
        break;
    case MoveDef::FunctionCode::TwoTurnAttackOneTurnInSun:
        break;
    default:
        break;
    }

    power = std::max<size_t>(1, static_cast<size_t>(static_cast<float>(power) * ItemUtils::heldPowerMultiplier(ctx.user, ctx.def, &ctx.userBattler)));
    power = std::max<size_t>(1, static_cast<size_t>(static_cast<float>(power)
            * abilityPowerMultiplier(ctx.user, ctx.userBattler, ctx.def, ctx.data.fieldWeather())));
    return std::max<size_t>(1, power);
}

bool applyMoveFunctionCode(MoveEffectContext& ctx)
{
    using FC = MoveDef::FunctionCode;
    auto code = ctx.def->getFunctionCode();

    // Rest
    if (code == FC::HealUserFullyAndFallAsleep)
    {
        if (abilityBlocksStatus(ctx.user, ctx.userBattler, Pkmn::StatusCondition::SLEEP))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.user->resetHP();
        ctx.user->setStatusCondition(Pkmn::StatusCondition::NONE);
        if (ctx.data.playerBattler.uproarTurns > 0 || ctx.data.opponentBattler.uproarTurns > 0)
            msgFmt(ctx, boost::format(lc::translate("%1% restored its HP, but the uproar kept it awake!")) % ctx.user->getDisplayName());
        else
        {
            applyStatusCondition(ctx.user, Pkmn::StatusCondition::SLEEP, ctx.userBattler.toxicCounter);
            ctx.user->setStatusTurns(3);
            msgFmt(ctx, boost::format(lc::translate("%1% went to sleep and became healthy!")) % ctx.user->getDisplayName());
        }
        return true;
    }

    // Two-turn charging first turn (Sleep Talk / Power Herb / sun skip via instantTwoTurn).
    if (isTwoTurnChargingMove(code) && !ctx.userBattler.charging && !ctx.instantTwoTurn)
    {
        ctx.userBattler.charging = true;
        if (code == FC::TwoTurnAttackInvulnerableInSky
            || code == FC::TwoTurnAttackInvulnerableInSkyParalyzeTarget
            || code == FC::TwoTurnAttackInvulnerableInSkyTargetCannotAct)
            setHide(ctx.userBattler, BattleBattler::SemiKind::Sky);
        if (code == FC::TwoTurnAttackInvulnerableInSkyTargetCannotAct && ctx.target)
        {
            ctx.targetBattler.skyDropLocked = true;
            setHide(ctx.targetBattler, BattleBattler::SemiKind::Sky);
        }
        if (code == FC::TwoTurnAttackInvulnerableUnderground)
            setHide(ctx.userBattler, BattleBattler::SemiKind::Underground);
        if (code == FC::TwoTurnAttackInvulnerableUnderwater)
            setHide(ctx.userBattler, BattleBattler::SemiKind::Underwater);
        if (code == FC::TwoTurnAttackChargeRaiseUserDefense1)
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        if (code == FC::TwoTurnAttackChargeRaiseUserSpAtk1)
            changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        msgFmt(ctx, boost::format(lc::translate("%1% is glowing!")) % ctx.user->getDisplayName());
        return true; // charging — no hit this turn (resolver must skip damage)
    }
    if (ctx.instantTwoTurn && isTwoTurnChargingMove(code) && !ctx.userBattler.charging)
    {
        // Charge-turn stat boosts still apply when finishing in one action.
        if (code == FC::TwoTurnAttackChargeRaiseUserDefense1)
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        if (code == FC::TwoTurnAttackChargeRaiseUserSpAtk1)
            changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
    }
    if (ctx.userBattler.charging)
    {
        ctx.userBattler.charging = false;
        setHide(ctx.userBattler, BattleBattler::SemiKind::None);
    }

    if (isRechargeMove(code) && ctx.moveHit)
        ctx.userBattler.recharging = true;

    switch (code)
    {
    case FC::None:
        return true;

    // --- Stat raises (user) ---
    // Damaging moves that share these FCs (Charge Beam, Meteor Mash, …) must roll
    // effectChance; Sheer Force suppresses via rollSecondary. Status setups always apply.
    case FC::RaiseUserAttack1:
        if (ctx.def->getCategory() != MoveDef::STATUS
            && (ctx.damageDealt == 0 || !rollSecondary(ctx)))
            return true;
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        return true;
    case FC::RaiseUserAttack2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 2, false);
        return true;
    case FC::RaiseUserDefense1:
        if (ctx.def->getCategory() != MoveDef::STATUS
            && (ctx.damageDealt == 0 || !rollSecondary(ctx)))
            return true;
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        return true;
    case FC::RaiseUserDefense2:
        if (ctx.def->getCategory() != MoveDef::STATUS
            && (ctx.damageDealt == 0 || !rollSecondary(ctx)))
            return true;
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 2, false);
        return true;
    case FC::RaiseUserDefense3:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 3, false);
        return true;
    case FC::RaiseUserSpeed1:
        if (ctx.def->getCategory() != MoveDef::STATUS
            && (ctx.damageDealt == 0 || !rollSecondary(ctx)))
            return true;
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::RaiseUserSpeed2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 2, false);
        return true;
    case FC::RaiseUserSpAtk1:
        if (ctx.def->getCategory() != MoveDef::STATUS
            && (ctx.damageDealt == 0 || !rollSecondary(ctx)))
            return true;
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        return true;
    case FC::RaiseUserSpAtk2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 2, false);
        return true;
    case FC::RaiseUserSpAtk3:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 2, false);
        return true;
    case FC::RaiseUserSpDef2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 2, false);
        return true;
    case FC::RaiseUserEvasion1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Evasion, 1, false);
        return true;
    case FC::RaiseUserCriticalHitRate2:
        if (ctx.userBattler.focusEnergy != 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.userBattler.focusEnergy = 2;
        msgFmt(ctx, boost::format(lc::translate("%1% is getting pumped!")) % ctx.user->getDisplayName());
        return true;
    case FC::LowerTargetAttack1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        return true;
    case FC::LowerTargetAttack2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -2, true);
        return true;
    case FC::LowerTargetDefense1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Defense, -1, true);
        return true;
    case FC::LowerTargetDefense2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Defense, -2, true);
        return true;
    case FC::LowerTargetSpeed1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -1, true);
        return true;
    case FC::LowerTargetSpeed2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -2, true);
        return true;
    case FC::LowerTargetSpAtk1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -1, true);
        return true;
    case FC::LowerTargetSpAtk2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -2, true);
        return true;
    case FC::LowerTargetSpDef1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialDefense, -1, true);
        return true;
    case FC::LowerTargetSpDef2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialDefense, -2, true);
        return true;
    case FC::LowerTargetAccuracy1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Accuracy, -1, true);
        return true;
    case FC::LowerTargetEvasion2:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Evasion, -2, true);
        return true;
    case FC::RaiseTargetAttack1:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, 1, true);
        return true;
    case FC::RaiseTargetSpDef1:
        // Aromatic Mist is ally-only; fails in singles.
        msg(ctx, lc::translate("But it failed!"));
        return false;

    case FC::RaiseUserAtkDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        return true;
    case FC::RaiseUserAtkSpAtk1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        return true;
    case FC::RaiseUserAtkSpd1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::RaiseUserAtkAcc1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Accuracy, 1, false);
        return true;
    case FC::RaiseUserAtk1Spd2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 2, false);
        return true;
    case FC::RaiseUserDefSpDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        return true;
    case FC::RaiseUserSpAtkSpDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        return true;
    case FC::RaiseUserSpAtkSpDefSpd1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::RaiseUserMainStats1:
        // Ancient Power / Silver Wind / Ominous Wind — damaging-only secondary.
        if (ctx.damageDealt == 0 || !rollSecondary(ctx))
            return true;
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::RaiseUserMainStats1LoseThirdOfTotalHP: {
        size_t maxHp = maxHpOf(ctx.user);
        if (ctx.user->getHP() * 3 <= maxHp)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.user->decreaseHP(std::max<size_t>(1, maxHp / 3));
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    }
    case FC::RaiseUserDefense1CurlUpUser:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        ctx.userBattler.defenseCurl = true;
        return true;
    case FC::RaiseUserEvasion2MinimizeUser:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Evasion, 2, false);
        ctx.userBattler.minimized = true;
        return true;
    case FC::RaiseUserSpeed2LowerUserWeight:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 2, false);
        ctx.userBattler.weightFactor *= 0.5f;
        return true;
    case FC::RaiseUserAtkDefAcc1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Accuracy, 1, false);
        return true;
    case FC::LowerUserAtkDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        return true;
    case FC::LowerUserDefense1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        return true;
    case FC::LowerUserDefSpDef1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, -1, false);
        return true;
    case FC::LowerUserSpAtk2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, -2, false);
        return true;
    case FC::LowerUserSpeed1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, -1, false);
        return true;
    case FC::LowerUserDefSpDefSpd1:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, -1, false);
        return true;
    case FC::LowerUserDefSpDef1RaiseUserAtkSpAtkSpd2:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 2, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 2, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 2, false);
        return true;
    case FC::LowerTargetAtkDef1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Defense, -1, true);
        return true;
    case FC::LowerTargetAtkSpAtk1:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -1, true);
        return true;
    case FC::LowerTargetAtkSpAtk1SwitchOutUser: { // Parting Shot
        bool atk = changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        bool spa = changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -1, true);
        if (!atk && !spa)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        if (sideHasReplacement(ctx, ctx.playerIsUser))
            ctx.shouldSwitchUser = true;
        return true;
    }
    case FC::LowerTargetAttack1BypassSubstitute:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
        return true;
    case FC::LowerTargetEvasion1RemoveSideEffects:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Evasion, -1, true);
        ctx.data.playerSpikes = ctx.data.opponentSpikes = 0;
        if (ctx.playerIsUser)
        {
            ctx.data.opponentReflect = ctx.data.opponentLightScreen = ctx.data.opponentAuroraVeil = 0;
            ctx.data.opponentSafeguard = ctx.data.opponentMist = 0;
        }
        else
        {
            ctx.data.playerReflect = ctx.data.playerLightScreen = ctx.data.playerAuroraVeil = 0;
            ctx.data.playerSafeguard = ctx.data.playerMist = 0;
        }
        return true;
    case FC::ResetTargetStatStages:
        ctx.targetBattler.resetStages();
        msgFmt(ctx, boost::format(lc::translate("%1%'s stat changes were removed!")) % ctx.target->getDisplayName());
        return true;
    case FC::ResetAllBattlersStatStages:
        ctx.data.playerBattler.resetStages();
        ctx.data.opponentBattler.resetStages();
        msg(ctx, lc::translate("All stat changes were eliminated!"));
        return true;
    case FC::InvertTargetStatStages:
        ctx.targetBattler.invertStages();
        msgFmt(ctx, boost::format(lc::translate("%1%'s stat changes were inverted!")) % ctx.target->getDisplayName());
        return true;
    case FC::UserCopyTargetStatStages:
        ctx.userBattler.copyStagesFrom(ctx.targetBattler);
        msgFmt(ctx, boost::format(lc::translate("%1% copied %2%'s stat changes!")) % ctx.user->getDisplayName()
                         % ctx.target->getDisplayName());
        return true;
    case FC::UserTargetSwapStatStages:
        ctx.userBattler.swapStagesWith(ctx.targetBattler);
        msg(ctx, lc::translate("The battlers swapped stat changes!"));
        return true;
    case FC::UserTargetSwapAtkSpAtkStages:
    {
        std::swap(ctx.userBattler.stages[static_cast<size_t>(SS::Attack)],
                  ctx.targetBattler.stages[static_cast<size_t>(SS::Attack)]);
        std::swap(ctx.userBattler.stages[static_cast<size_t>(SS::SpecialAttack)],
                  ctx.targetBattler.stages[static_cast<size_t>(SS::SpecialAttack)]);
        msg(ctx, lc::translate("The battlers swapped their Attack and Sp. Atk changes!"));
        return true;
    }
    case FC::UserTargetSwapDefSpDefStages:
    {
        std::swap(ctx.userBattler.stages[static_cast<size_t>(SS::Defense)],
                  ctx.targetBattler.stages[static_cast<size_t>(SS::Defense)]);
        std::swap(ctx.userBattler.stages[static_cast<size_t>(SS::SpecialDefense)],
                  ctx.targetBattler.stages[static_cast<size_t>(SS::SpecialDefense)]);
        msg(ctx, lc::translate("The battlers swapped their Defense and Sp. Def changes!"));
        return true;
    }
    case FC::RaiseTargetAttack2ConfuseTarget:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, 2, false);
        {
            ConfusionStatus& conf = ctx.playerIsUser ? ctx.data.opponentConfusion : ctx.data.playerConfusion;
            if (!blocksConfusion(ctx) && conf.apply())
                msgFmt(ctx, boost::format(lc::translate("%1% became confused!")) % ctx.target->getDisplayName());
        }
        return true;
    case FC::RaiseTargetSpAtk1ConfuseTarget:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, 1, false);
        {
            ConfusionStatus& conf = ctx.playerIsUser ? ctx.data.opponentConfusion : ctx.data.playerConfusion;
            if (!blocksConfusion(ctx) && conf.apply())
                msgFmt(ctx, boost::format(lc::translate("%1% became confused!")) % ctx.target->getDisplayName());
        }
        return true;

    // --- Status infliction ---
    case FC::BurnTargetIfTargetStatsRaisedThisTurn:
        if (!ctx.targetBattler.statsRaisedThisTurn)
        {
            if (ctx.def->getCategory() == MoveDef::STATUS)
            {
                msg(ctx, lc::translate("But it failed!"));
                return false;
            }
            return true;
        }
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx))
            return true;
        tryStatus(ctx, Pkmn::StatusCondition::BURN);
        return true;
    case FC::BurnTarget:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::BURN);
        return true;
    case FC::BurnFlinchTarget:
        if (rollSecondary(ctx))
        {
            tryStatus(ctx, Pkmn::StatusCondition::BURN);
            tryFlinch(ctx);
        }
        return true;
    case FC::FreezeTarget:
    case FC::FreezeTargetAlwaysHitsInHail:
    case FC::FreezeTargetSuperEffectiveAgainstWater:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::FREEZE);
        return true;
    case FC::FreezeFlinchTarget:
        if (rollSecondary(ctx))
        {
            tryStatus(ctx, Pkmn::StatusCondition::FREEZE);
            tryFlinch(ctx);
        }
        return true;
    case FC::ParalyzeTargetIfNotTypeImmune:
        if (ctx.target && hasType(ctx.target, "GROUND", &ctx.targetBattler))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx))
            return true;
        tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
        return true;
    case FC::ParalyzeTarget:
    case FC::ParalyzeTargetAlwaysHitsInRainHitsTargetInSky:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
        return true;
    case FC::ParalyzeFlinchTarget:
        if (rollSecondary(ctx))
        {
            tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
            tryFlinch(ctx);
        }
        return true;
    case FC::PoisonTarget:
    case FC::PoisonTargetLowerTargetSpeed1:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::POISON);
        if (code == FC::PoisonTargetLowerTargetSpeed1)
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -1, true);
        return true;
    case FC::HitTwoTimesPoisonTarget:
        if (rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::POISON);
        return true;
    case FC::BadPoisonTarget:
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        tryStatus(ctx, Pkmn::StatusCondition::BADLY_POISON);
        return true;
    case FC::SleepTargetIfUserDarkrai:
        if (!ctx.user || !ctx.user->getDefinition() || ctx.user->getDefinition()->getId() != "DARKRAI")
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        tryStatus(ctx, Pkmn::StatusCondition::SLEEP);
        return true;
    case FC::SleepTarget:
        tryStatus(ctx, Pkmn::StatusCondition::SLEEP);
        return true;
    case FC::SleepTargetChangeUserMeloettaForm:
        if (rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::SLEEP);
        if (ctx.damageDealt > 0 && ctx.user && ctx.user->getDefinition() && ctx.user->getDefinition()->getId() == "MELOETTA")
        {
            ctx.userBattler.pirouette = !ctx.userBattler.pirouette;
            if (ctx.userBattler.pirouette)
            {
                ctx.userBattler.typesOverride      = {"NORMAL", "FIGHTING"};
                ctx.userBattler.attackOverride    = static_cast<int>(ctx.user->statFromBase(PkmnDef::ATTACK, 128));
                ctx.userBattler.defenseOverride   = static_cast<int>(ctx.user->statFromBase(PkmnDef::DEFENSE, 90));
                ctx.userBattler.spAttackOverride  = static_cast<int>(ctx.user->statFromBase(PkmnDef::SPECIAL_ATTACK, 77));
                ctx.userBattler.spDefenseOverride = static_cast<int>(ctx.user->statFromBase(PkmnDef::SPECIAL_DEFENSE, 77));
                ctx.userBattler.speedOverride     = static_cast<int>(ctx.user->statFromBase(PkmnDef::SPEED, 128));
            }
            else
            {
                ctx.userBattler.typesOverride.clear();
                ctx.userBattler.attackOverride    = -1;
                ctx.userBattler.defenseOverride   = -1;
                ctx.userBattler.spAttackOverride  = -1;
                ctx.userBattler.spDefenseOverride = -1;
                ctx.userBattler.speedOverride     = -1;
            }
            msgFmt(ctx, boost::format(lc::translate("%1% transformed!")) % ctx.user->getDisplayName());
        }
        return true;
    case FC::SleepTargetNextTurn:
        if (!ctx.target || ctx.targetBattler.yawnTurns > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.yawnTurns = 2;
        msgFmt(ctx, boost::format(lc::translate("%1% grew drowsy!")) % ctx.target->getDisplayName());
        return true;
    case FC::ParalyzeBurnOrFreezeTarget:
        if (!rollSecondary(ctx)) return true;
        switch (Utils::randuint(0, 2))
        {
        case 0: tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS); break;
        case 1: tryStatus(ctx, Pkmn::StatusCondition::BURN); break;
        default: tryStatus(ctx, Pkmn::StatusCondition::FREEZE); break;
        }
        return true;
    case FC::ConfuseTarget:
    case FC::ConfuseTargetAlwaysHitsInRainHitsTargetInSky:
    {
        if (ctx.def->getCategory() != MoveDef::STATUS && !rollSecondary(ctx)) return true;
        ConfusionStatus& conf = ctx.playerIsUser ? ctx.data.opponentConfusion : ctx.data.playerConfusion;
        if (blocksConfusion(ctx))
            return true;
        if (conf.apply())
            msgFmt(ctx, boost::format(lc::translate("%1% became confused!")) % ctx.target->getDisplayName());
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    }
    case FC::AttractTarget:
        if (!ctx.target || ctx.target->getGender() == Pkmn::Gender::GENDERLESS
            || ctx.user->getGender() == Pkmn::Gender::GENDERLESS
            || ctx.target->getGender() == ctx.user->getGender() || ctx.targetBattler.infatuated
            || aromaVeil(ctx)
            || oblivious(ctx))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.infatuated = true;
        if (ItemUtils::activeHeldId(ctx.target, &ctx.targetBattler) == "DESTINYKNOT")
            ctx.userBattler.infatuated = true;
        msgFmt(ctx, boost::format(lc::translate("%1% fell in love!")) % ctx.target->getDisplayName());
        ItemUtils::consumeMentalHerb(ctx.target, &ctx.targetBattler, ctx.messages);
        ItemUtils::consumeMentalHerb(ctx.user, &ctx.userBattler, ctx.messages);
        return true;

    // --- Flinch / Protect ---
    case FC::FlinchTargetFailsIfUserNotAsleep:
        if (!ctx.user || ctx.user->getStatusCondition() != Pkmn::StatusCondition::SLEEP)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        tryFlinch(ctx);
        return true;
    case FC::FlinchTarget:
    case FC::FlinchTargetDoublePowerIfTargetInSky:
    case FC::TwoTurnAttackFlinchTarget:
        if (rollSecondary(ctx) || ctx.def->getEffectChance() == 0 || ctx.def->getEffectChance() == 100)
            tryFlinch(ctx);
        return true;
    case FC::FlinchTargetFailsIfNotUserFirstTurn:
        if (ctx.userBattler.activeMoveActions > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        tryFlinch(ctx);
        return true;
    case FC::ProtectUser:
        if (!protectSucceeds(ctx))
            return false;
        ctx.userBattler.protect     = true;
        ctx.userBattler.protectKind = BattleBattler::ProtectKind::Normal;
        msgFmt(ctx, boost::format(lc::translate("%1% protected itself!")) % ctx.user->getDisplayName());
        return true;
    case FC::ProtectUserBanefulBunker:
        if (!protectSucceeds(ctx))
            return false;
        ctx.userBattler.protect     = true;
        ctx.userBattler.protectKind = BattleBattler::ProtectKind::BanefulBunker;
        msgFmt(ctx, boost::format(lc::translate("%1% protected itself!")) % ctx.user->getDisplayName());
        return true;
    case FC::ProtectUserFromDamagingMovesKingsShield:
        if (!protectSucceeds(ctx))
            return false;
        ctx.userBattler.protect     = true;
        ctx.userBattler.protectKind = BattleBattler::ProtectKind::KingsShield;
        if (!ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "STANCECHANGE")
        {
            ctx.userBattler.bladeForme        = false;
            ctx.userBattler.attackOverride    = -1;
            ctx.userBattler.defenseOverride   = -1;
            ctx.userBattler.spAttackOverride  = -1;
            ctx.userBattler.spDefenseOverride = -1;
        }
        msgFmt(ctx, boost::format(lc::translate("%1% protected itself!")) % ctx.user->getDisplayName());
        return true;
    case FC::ProtectUserFromDamagingMovesObstruct:
        if (!protectSucceeds(ctx))
            return false;
        ctx.userBattler.protect     = true;
        ctx.userBattler.protectKind = BattleBattler::ProtectKind::Obstruct;
        msgFmt(ctx, boost::format(lc::translate("%1% protected itself!")) % ctx.user->getDisplayName());
        return true;
    case FC::ProtectUserFromTargetingMovesSpikyShield:
        if (!protectSucceeds(ctx))
            return false;
        ctx.userBattler.protect     = true;
        ctx.userBattler.protectKind = BattleBattler::ProtectKind::SpikyShield;
        msgFmt(ctx, boost::format(lc::translate("%1% protected itself!")) % ctx.user->getDisplayName());
        return true;
    case FC::ProtectUserSideFromMultiTargetDamagingMoves:
        if (!protectSucceeds(ctx))
            return false;
        (ctx.playerIsUser ? ctx.data.playerWideGuard : ctx.data.opponentWideGuard) = true;
        msg(ctx, lc::translate("A wide guard protected the team!"));
        return true;
    case FC::ProtectUserSideFromPriorityMoves:
        if (!protectSucceeds(ctx))
            return false;
        (ctx.playerIsUser ? ctx.data.playerQuickGuard : ctx.data.opponentQuickGuard) = true;
        msg(ctx, lc::translate("Quick Guard protected the team!"));
        return true;
    case FC::ProtectUserSideFromStatusMoves:
        // Crafty Shield does not use / consume the Protect streak.
        (ctx.playerIsUser ? ctx.data.playerCraftyShield : ctx.data.opponentCraftyShield) = true;
        msg(ctx, lc::translate("Crafty Shield protected the team!"));
        return true;
    case FC::ProtectUserSideFromDamagingMovesIfUserFirstTurn:
        if (!ctx.userBattler.switchedInThisTurn)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        // Mat Block does not use / consume the Protect streak.
        (ctx.playerIsUser ? ctx.data.playerMatBlock : ctx.data.opponentMatBlock) = true;
        msg(ctx, lc::translate("Mat Block protected the team!"));
        return true;
    case FC::UserEnduresFaintingThisTurn:
        if (!protectSucceeds(ctx))
            return false;
        ctx.userBattler.endure = true;
        msgFmt(ctx, boost::format(lc::translate("%1% braced itself!")) % ctx.user->getDisplayName());
        return true;
    case FC::UserMakeSubstitute:
    {
        size_t cost = std::max<size_t>(1, maxHpOf(ctx.user) / 4);
        if (ctx.user->getHP() <= cost || ctx.userBattler.substituteHP > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.user->decreaseHP(cost);
        ctx.userBattler.substituteHP = static_cast<int>(cost);
        msgFmt(ctx, boost::format(lc::translate("%1% put up a substitute!")) % ctx.user->getDisplayName());
        return true;
    }

    // --- Healing ---
    case FC::HealUserHalfOfTotalHP:
        healUser(ctx, maxHpOf(ctx.user) / 2);
        return true;
    case FC::HealUserHalfOfTotalHPLoseFlyingTypeThisTurn:
    {
        healUser(ctx, maxHpOf(ctx.user) / 2);
        ctx.userBattler.roostTypes = ctx.userBattler.typesOverride;
        auto types = activeTypes(ctx.user, &ctx.userBattler);
        types.erase(std::remove_if(types.begin(), types.end(),
                                   [](std::string const& type) { return type == "FLYING"; }),
                    types.end());
        if (types.empty())
            types.push_back("NORMAL");
        ctx.userBattler.typesOverride = std::move(types);
        ctx.userBattler.roosting      = true;
        return true;
    }
    case FC::HealUserDependingOnWeather:
    {
        size_t amount = maxHpOf(ctx.user) / 2;
        bool umbrella = ItemUtils::activeHeldId(ctx.user, &ctx.userBattler) == "UTILITYUMBRELLA";
        if (!umbrella && ctx.data.fieldWeather() == Map::Weather::HARSH_SUNLIGHT)
            amount = maxHpOf(ctx.user) * 2 / 3;
        else if (ctx.data.fieldWeather() == Map::Weather::SANDSTORM || ctx.data.fieldWeather() == Map::Weather::HAIL
                 || (!umbrella && ctx.data.fieldWeather() == Map::Weather::RAIN))
            amount = maxHpOf(ctx.user) / 4;
        healUser(ctx, amount);
        return true;
    }
    case FC::HealUserByHalfOfDamageDone:
    case FC::HealUserByHalfOfDamageDoneIfTargetAsleep:
    case FC::HealUserByThreeQuartersOfDamageDone:
    {
        if (ctx.damageDealt == 0)
            return true;
        size_t amount = ctx.damageDealt / 2;
        if (code == FC::HealUserByThreeQuartersOfDamageDone)
            amount = ctx.damageDealt * 3 / 4;
        if (ItemUtils::activeHeldId(ctx.user, &ctx.userBattler) == "BIGROOT")
            amount = std::max<size_t>(1, amount * 13 / 10);
        if (!ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "LIQUIDOOZE" && ctx.user)
        {
            if (ctx.userBattler.abilitySuppressed || ctx.userBattler.abilityId != "MAGICGUARD")
            {
                ctx.user->decreaseHP(std::max<size_t>(1, amount));
                msgFmt(ctx, boost::format(lc::translate("%1% was hurt by %2%'s Liquid Ooze!")) % ctx.user->getDisplayName()
                                 % ctx.target->getDisplayName());
            }
            return true;
        }
        healUser(ctx, std::max<size_t>(1, amount));
        return true;
    }
    case FC::HealTargetHalfOfTotalHP:
    {
        if (ctx.targetBattler.healBlockTurns > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        size_t amount = maxHpOf(ctx.target) / 2;
        size_t newHp  = std::min(maxHpOf(ctx.target), ctx.target->getHP() + amount);
        ctx.target->setHP(newHp);
        msgFmt(ctx, boost::format(lc::translate("%1% had its HP restored.")) % ctx.target->getDisplayName());
        return true;
    }
    case FC::CureUserBurnPoisonParalysis:
        if (ctx.user->getStatusCondition() == Pkmn::StatusCondition::BURN
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::POISON
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON
            || ctx.user->getStatusCondition() == Pkmn::StatusCondition::PARALYSIS)
        {
            ctx.user->setStatusCondition(Pkmn::StatusCondition::NONE);
            msgFmt(ctx, boost::format(lc::translate("%1%'s status returned to normal!")) % ctx.user->getDisplayName());
        }
        return true;
    case FC::CureTargetBurn:
        if (ctx.damageDealt == 0)
            return true;
        if (ctx.target->getStatusCondition() == Pkmn::StatusCondition::BURN)
        {
            ctx.target->setStatusCondition(Pkmn::StatusCondition::NONE);
            msgFmt(ctx, boost::format(lc::translate("%1%'s burn was healed!")) % ctx.target->getDisplayName());
        }
        return true;
    case FC::CureUserPartyStatus:
    {
        bool healBell = ctx.def && ctx.def->getId() == "HEALBELL";
        auto soundproof = [&](Pkmn::PkmnPtr const& mon) {
            if (!mon)
                return false;
            if (mon == ctx.user)
                return !ctx.userBattler.abilitySuppressed && ctx.userBattler.abilityId == "SOUNDPROOF";
            return mon->getAbilityId() == "SOUNDPROOF";
        };
        auto cure = [&](Pkmn::PkmnPtr const& mon) {
            if (!mon || (healBell && soundproof(mon)))
                return;
            mon->setStatusCondition(Pkmn::StatusCondition::NONE);
        };
        if (ctx.playerIsUser)
        {
            for (auto const& mon : Game::instance()->data.player.pkmns)
                cure(mon);
        }
        else if (auto trainerData = dynamic_cast<TrainerSceneData*>(&ctx.data))
        {
            if (trainerData->opponentTrainer)
            {
                for (auto const& mon : trainerData->opponentTrainer->pkmns)
                    cure(mon);
            }
            else
                cure(ctx.user);
        }
        else
            cure(ctx.user);
        msg(ctx, lc::translate("A soothing aroma wafted through the area!"));
        return true;
    }

    // --- Recoil / OHKO / fixed damage ---
    case FC::RecoilQuarterOfDamageDealt:
        if (ctx.def && ctx.def->getId() == "STRUGGLE")
        {
            ctx.user->decreaseHP(std::max<size_t>(1, maxHpOf(ctx.user) / 4));
            ctx.userBattler.tookDamageThisTurn = true;
            return true;
        }
        if (ctx.damageDealt > 0 && !skipsRecoil(ctx.userBattler))
        {
            ctx.user->decreaseHP(std::max<size_t>(1, ctx.damageDealt / 4));
            ctx.userBattler.tookDamageThisTurn = true;
        }
        return true;
    case FC::RecoilThirdOfDamageDealt:
    case FC::RecoilThirdOfDamageDealtBurnTarget:
    case FC::RecoilThirdOfDamageDealtParalyzeTarget:
        if (ctx.damageDealt > 0 && !skipsRecoil(ctx.userBattler))
        {
            ctx.user->decreaseHP(std::max<size_t>(1, ctx.damageDealt / 3));
            ctx.userBattler.tookDamageThisTurn = true;
        }
        if (code == FC::RecoilThirdOfDamageDealtBurnTarget && rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::BURN);
        if (code == FC::RecoilThirdOfDamageDealtParalyzeTarget && rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
        return true;
    case FC::RecoilHalfOfDamageDealt:
        if (ctx.damageDealt > 0 && !skipsRecoil(ctx.userBattler))
        {
            ctx.user->decreaseHP(std::max<size_t>(1, ctx.damageDealt / 2));
            ctx.userBattler.tookDamageThisTurn = true;
        }
        return true;
    case FC::OHKO:
    case FC::OHKOIce:
    case FC::OHKOHitsUndergroundTarget: {
        if (!ctx.target || ctx.user->getLevel() < ctx.target->getLevel())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        if (ctx.targetBattler.substituteHP > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        bool ignoreAbility = !ctx.userBattler.abilitySuppressed
                          && (ctx.userBattler.abilityId == "MOLDBREAKER" || ctx.userBattler.abilityId == "TERAVOLT"
                              || ctx.userBattler.abilityId == "TURBOBLAZE");
        if (!ignoreAbility && !ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "STURDY"
            && ctx.target->getHP() > 0)
        {
            msgFmt(ctx, boost::format(lc::translate("%1%'s Sturdy endured the hit!")) % ctx.target->getDisplayName());
            return false;
        }
        if (!applyScriptedDamage(ctx, ctx.target->getHP(), true))
            return false;
        if (ctx.target->isKO())
            msg(ctx, lc::translate("It's a one-hit KO!"));
        return true;
    }
    case FC::FixedDamage20:
        return applyScriptedDamage(ctx, 20, false);
    case FC::FixedDamage40:
        return applyScriptedDamage(ctx, 40, false);
    case FC::FixedDamageHalfTargetHP:
        return applyScriptedDamage(ctx, std::max<size_t>(1, ctx.target->getHP() / 2), false);
    case FC::FixedDamageUserLevel:
        return applyScriptedDamage(ctx, std::max<size_t>(1, ctx.user->getLevel()), false);
    case FC::FixedDamageUserLevelRandom: {
        size_t level = std::max<size_t>(1, ctx.user->getLevel());
        size_t dealt = std::max<size_t>(1, level * Utils::randuint(50, 150) / 100);
        return applyScriptedDamage(ctx, dealt, false);
    }
    case FC::LowerTargetHPToUserHP:
        if (!ctx.target || ctx.target->getHP() <= ctx.user->getHP())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        return applyScriptedDamage(ctx, ctx.target->getHP() - ctx.user->getHP(), false);
    case FC::UserLosesHalfOfTotalHP:
    case FC::UserLosesHalfOfTotalHPExplosive:
        if (ctx.userBattler.abilitySuppressed || ctx.userBattler.abilityId != "MAGICGUARD")
        {
            ctx.user->decreaseHP(std::max<size_t>(1, (maxHpOf(ctx.user) + 1) / 2));
            ctx.userBattler.tookDamageThisTurn = true;
        }
        return true;
    case FC::UserFaintsFixedDamageUserHP: {
        size_t dealt = ctx.user->getHP();
        if (ctx.target && dealt > 0)
            applyScriptedDamage(ctx, dealt, false);
        ctx.user->setHP(0);
        return true;
    }
    case FC::UserFaintsExplosive:
    case FC::UserFaintsLowerTargetAtkSpAtk2:
    case FC::UserFaintsPowersUpInMistyTerrainExplosive:
        ctx.user->setHP(0);
        if (code == FC::UserFaintsLowerTargetAtkSpAtk2)
        {
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -2, true);
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -2, true);
        }
        return true;
    case FC::CrashDamageIfFailsUnusableInGravity:
        // applied by resolver on miss
        return true;

    // --- Multi-turn / binding ---
    case FC::BindTarget:
    case FC::BindTargetDoublePowerIfTargetUnderwater: {
        if (ctx.damageDealt == 0 || ctx.hitSubstitute)
            return true;
        ctx.targetBattler.bindingTurns = static_cast<int>(Utils::randuint(2, 5));
        ctx.targetBattler.bindingBand  = false;
        ctx.targetBattler.trapped      = true;
        msgFmt(ctx, boost::format(lc::translate("%1% was trapped!")) % ctx.target->getDisplayName());
        return true;
    }
    case FC::TrapTargetInBattle:
    case FC::TrapTargetInBattleMainEffect: {
        bool statusTrap = ctx.def && ctx.def->getCategory() == MoveDef::STATUS;
        // Mean Look / Block / Spider Web: no damage required; fail vs Sub / Ghost.
        // Anchor Shot / Spirit Shackle: need a real HP hit past Substitute (Ghosts OK).
        // Thousand Waves (MainEffect): damaging; Sub blocks; Ghosts OK.
        if (statusTrap)
        {
            if (ctx.hitSubstitute || !ctx.target)
            {
                msg(ctx, lc::translate("But it failed!"));
                return false;
            }
        }
        else if (code == FC::TrapTargetInBattle && (ctx.damageDealt == 0 || ctx.hitSubstitute))
            return true;
        else if (code == FC::TrapTargetInBattleMainEffect && ctx.hitSubstitute)
            return true;
        ctx.targetBattler.trapped    = true;
        ctx.targetBattler.meanLooked = true;
        msgFmt(ctx, boost::format(lc::translate("%1% can no longer escape!")) % ctx.target->getDisplayName());
        return true;
    }
    case FC::MultiTurnAttackPreventSleeping:
        if (ctx.userBattler.uproarTurns <= 0)
        {
            ctx.userBattler.uproarTurns  = static_cast<int>(Utils::randuint(2, 5));
            ctx.userBattler.uproarMoveId = ctx.def->getId();
            msgFmt(ctx, boost::format(lc::translate("%1% caused an uproar!")) % ctx.user->getDisplayName());
        }
        return true;
    case FC::MultiTurnAttackConfuseUserAtEnd:
        if (ctx.userBattler.multiTurnLock <= 0)
        {
            ctx.userBattler.multiTurnLock       = static_cast<int>(Utils::randuint(2, 3));
            ctx.userBattler.rampageMoveId       = ctx.def->getId();
            ctx.userBattler.rampageConfuseAtEnd = true;
            msgFmt(ctx, boost::format(lc::translate("%1% started a rampage!")) % ctx.user->getDisplayName());
        }
        return true;
    case FC::MultiTurnAttackPowersUpEachTurn:
        if (ctx.userBattler.multiTurnLock <= 0)
        {
            ctx.userBattler.multiTurnLock = 5;
            ctx.userBattler.rolloutMoveId = ctx.def->getId();
            msgFmt(ctx, boost::format(lc::translate("%1% started rolling!")) % ctx.user->getDisplayName());
        }
        return true;
    case FC::AttackAndSkipNextTurn:
        return true;
    case FC::StartLeechSeedTarget:
        if (hasType(ctx.target, "GRASS", &ctx.targetBattler) || ctx.targetBattler.seeded)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.seeded = true;
        msgFmt(ctx, boost::format(lc::translate("%1% was seeded!")) % ctx.target->getDisplayName());
        return true;

    // --- Hazards ---
    case FC::AddSpikesToFoeSide:
    {
        int& spikes = ctx.playerIsUser ? ctx.data.opponentSpikes : ctx.data.playerSpikes;
        if (spikes > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        spikes = 1;
        msg(ctx, lc::translate("Spikes were scattered all around the opposing team's feet!"));
        return true;
    }
    case FC::AddToxicSpikesToFoeSide:
    case FC::AddStealthRocksToFoeSide:
    case FC::AddStickyWebToFoeSide:
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::RemoveUserBindingAndEntryHazards:
        ctx.userBattler.bindingTurns = 0;
        if (!ctx.userBattler.meanLooked && !ctx.userBattler.octolock && !ctx.userBattler.jawLocked
            && !ctx.userBattler.ingrain && !ctx.userBattler.noRetreat)
            ctx.userBattler.trapped = false;
        if (ctx.playerIsUser)
            ctx.data.playerSpikes = 0;
        else
            ctx.data.opponentSpikes = 0;
        msg(ctx, lc::translate("The effects of hazards disappeared!"));
        return true;

    // --- Weather / field ---
    case FC::StartRainWeather:
        if (primordialWeatherBlocksNewWeather(ctx.data))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.data.weather         = Map::Weather::RAIN;
        ctx.data.weatherFromMove = true;
        ctx.data.weatherTurns    = ItemUtils::weatherTurnsForHolder(ctx.user, ctx.data.weather, &ctx.userBattler);
        msg(ctx, lc::translate("It started to rain!"));
        return true;
    case FC::StartSunWeather:
        if (primordialWeatherBlocksNewWeather(ctx.data))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.data.weather         = Map::Weather::HARSH_SUNLIGHT;
        ctx.data.weatherFromMove = true;
        ctx.data.weatherTurns    = ItemUtils::weatherTurnsForHolder(ctx.user, ctx.data.weather, &ctx.userBattler);
        msg(ctx, lc::translate("The sunlight turned harsh!"));
        return true;
    case FC::StartSandstormWeather:
        if (primordialWeatherBlocksNewWeather(ctx.data))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.data.weather         = Map::Weather::SANDSTORM;
        ctx.data.weatherFromMove = true;
        ctx.data.weatherTurns    = ItemUtils::weatherTurnsForHolder(ctx.user, ctx.data.weather, &ctx.userBattler);
        msg(ctx, lc::translate("A sandstorm kicked up!"));
        return true;
    case FC::StartHailWeather:
        if (primordialWeatherBlocksNewWeather(ctx.data))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.data.weather         = Map::Weather::HAIL;
        ctx.data.weatherFromMove = true;
        ctx.data.weatherTurns    = ItemUtils::weatherTurnsForHolder(ctx.user, ctx.data.weather, &ctx.userBattler);
        msg(ctx, lc::translate("It started to hail!"));
        return true;
    case FC::StartElectricTerrain:
    case FC::StartGrassyTerrain:
    case FC::StartMistyTerrain:
    case FC::StartPsychicTerrain:
    case FC::RemoveTerrain:
    case FC::StartGravity:
    case FC::StartSlowerBattlersActFirst:
        msg(ctx, lc::translate("But it failed!"));
        return false;

    // --- Switching ---
    case FC::SwitchOutUserDamagingMove:
        // U-turn / Volt Switch / Flip Turn: damage lands; switch only with a bench.
        if (sideHasReplacement(ctx, ctx.playerIsUser))
            ctx.shouldSwitchUser = true;
        return true;
    case FC::SwitchOutUserStatusMove:
        // Teleport flees a wild battle and fails against a trainer.
        if (ctx.playerIsUser && dynamic_cast<TrainerSceneData*>(&ctx.data) == nullptr)
        {
            ctx.data.forceWildEnd = true;
            return true;
        }
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::SwitchOutUserPassOnEffects: {
        if (!sideHasReplacement(ctx, ctx.playerIsUser))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.shouldSwitchUser = true;
        if (ctx.playerIsUser)
            ctx.data.playerVoluntarySwitch = true;
        else
            ctx.data.opponentVoluntarySwitch = true;
        BatonPassState& pass = ctx.playerIsUser ? ctx.data.playerBatonPass : ctx.data.opponentBatonPass;
        pass.pending          = true;
        pass.stages           = ctx.userBattler.stages;
        pass.substituteHP     = ctx.userBattler.substituteHP;
        pass.focusEnergy      = ctx.userBattler.focusEnergy;
        pass.perishCount      = ctx.userBattler.perishCount;
        pass.seeded           = ctx.userBattler.seeded;
        pass.ingrain          = ctx.userBattler.ingrain;
        pass.aquaRing         = ctx.userBattler.aquaRing;
        pass.confusion        = ctx.playerIsUser ? ctx.data.playerConfusion : ctx.data.opponentConfusion;
        pass.airborneTurns    = ctx.userBattler.airborneTurns;
        pass.telekinesis      = ctx.userBattler.telekinesis;
        pass.stockpile        = ctx.userBattler.stockpile;
        pass.stockpileDef     = ctx.userBattler.stockpileDef;
        pass.stockpileSpDef   = ctx.userBattler.stockpileSpDef;
        pass.attackOverride   = ctx.userBattler.attackOverride;
        pass.defenseOverride  = ctx.userBattler.defenseOverride;
        pass.spAttackOverride = ctx.userBattler.spAttackOverride;
        pass.spDefenseOverride = ctx.userBattler.spDefenseOverride;
        pass.speedOverride    = ctx.userBattler.speedOverride;
        return true;
    }
    case FC::SwitchOutTargetDamagingMove:
    case FC::SwitchOutTargetStatusMove:
        if (code == FC::SwitchOutTargetDamagingMove
            && (ctx.damageDealt == 0 || ctx.hitSubstitute))
            return true;
        if (!sideHasReplacement(ctx, !ctx.playerIsUser))
        {
            // Gen 7+: phazing a wild Pokémon with no bench ends the encounter.
            bool targetIsPlayer = !ctx.playerIsUser;
            bool wildTarget     = !targetIsPlayer && dynamic_cast<TrainerSceneData*>(&ctx.data) == nullptr;
            if (wildTarget)
            {
                ctx.data.forceWildEnd = true;
                return true;
            }
            if (code == FC::SwitchOutTargetStatusMove)
            {
                msg(ctx, lc::translate("But it failed!"));
                return false;
            }
            return true;
        }
        ctx.shouldSwitchTarget = true;
        ctx.forcedSwitchTarget = true;
        return true;

    // --- Screens / mist / safeguard ---
    case FC::StartWeakenPhysicalDamageAgainstUserSide: {
        int& turns = ctx.playerIsUser ? ctx.data.playerReflect : ctx.data.opponentReflect;
        if (turns > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        turns = ItemUtils::screenTurnsForHolder(ctx.user, &ctx.userBattler);
        msg(ctx, lc::translate("Reflect raised the team's Defense!"));
        return true;
    }
    case FC::StartWeakenSpecialDamageAgainstUserSide: {
        int& turns = ctx.playerIsUser ? ctx.data.playerLightScreen : ctx.data.opponentLightScreen;
        if (turns > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        turns = ItemUtils::screenTurnsForHolder(ctx.user, &ctx.userBattler);
        msg(ctx, lc::translate("Light Screen raised the team's Sp. Def!"));
        return true;
    }
    case FC::StartWeakenDamageAgainstUserSideIfHail: {
        if (ctx.data.fieldWeather() != Map::Weather::HAIL)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        int& turns = ctx.playerIsUser ? ctx.data.playerAuroraVeil : ctx.data.opponentAuroraVeil;
        if (turns > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        turns = ItemUtils::screenTurnsForHolder(ctx.user, &ctx.userBattler);
        msg(ctx, lc::translate("Aurora Veil made the team stronger against physical and special moves!"));
        return true;
    }
    case FC::StartUserSideImmunityToStatStageLowering: {
        int& mist = ctx.playerIsUser ? ctx.data.playerMist : ctx.data.opponentMist;
        if (mist > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        mist = 5;
        msgFmt(ctx, boost::format(lc::translate("%1%'s team became shrouded in mist!")) % ctx.user->getDisplayName());
        return true;
    }
    case FC::StartUserSideImmunityToInflictedStatus: {
        int& turns = ctx.playerIsUser ? ctx.data.playerSafeguard : ctx.data.opponentSafeguard;
        if (turns > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        turns = ItemUtils::screenTurnsForHolder(ctx.user, &ctx.userBattler);
        msg(ctx, lc::translate("The team became cloaked in a mystical veil!"));
        return true;
    }
    case FC::RemoveScreens:
        if (ctx.immune)
            return true;
        if (ctx.playerIsUser)
            ctx.data.opponentReflect = ctx.data.opponentLightScreen = ctx.data.opponentAuroraVeil = 0;
        else
            ctx.data.playerReflect = ctx.data.playerLightScreen = ctx.data.playerAuroraVeil = 0;
        msg(ctx, lc::translate("The opposing team's screens disappeared!"));
        return true;
    case FC::StartPerishCountsForAllBattlers: {
        auto applyPerish = [&](BattleBattler& battler) {
            if (battler.perishCount >= 0)
                return;
            if (!battler.abilitySuppressed && battler.abilityId == "SOUNDPROOF")
                return;
            battler.perishCount = 3;
        };
        applyPerish(ctx.data.playerBattler);
        applyPerish(ctx.data.opponentBattler);
        msg(ctx, lc::translate("All Pokémon will faint in three turns!"));
        return true;
    }
    case FC::StartHealUserEachTurn:
        ctx.userBattler.aquaRing = true;
        msgFmt(ctx, boost::format(lc::translate("%1% surrounded itself with a veil of water!")) % ctx.user->getDisplayName());
        return true;
    case FC::UserSwapBaseAtkDef: {
        int atk = ctx.userBattler.attackOverride >= 0 ? ctx.userBattler.attackOverride
                                                      : static_cast<int>(ctx.user->getStats().at(PkmnDef::ATTACK));
        int def = ctx.userBattler.defenseOverride >= 0 ? ctx.userBattler.defenseOverride
                                                       : static_cast<int>(ctx.user->getStats().at(PkmnDef::DEFENSE));
        ctx.userBattler.attackOverride  = def;
        ctx.userBattler.defenseOverride = atk;
        msgFmt(ctx, boost::format(lc::translate("%1% switched its Attack and Defense!")) % ctx.user->getDisplayName());
        return true;
    }
    case FC::TransformUserIntoTarget: {
        if (!ctx.target || ctx.target->isKO() || ctx.target == ctx.user || ctx.hitSubstitute
            || ctx.userBattler.transformActive)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        copyTransformFrom(ctx.user, ctx.userBattler, ctx.target, ctx.targetBattler);
        msgFmt(ctx, boost::format(lc::translate("%1% transformed into %2%!")) % ctx.user->getDisplayName()
                         % ctx.target->getDisplayName());
        return true;
    }
    case FC::DisableTargetLastMoveUsed:
        if (!ctx.target || aromaVeil(ctx) || ctx.targetBattler.lastMoveId.empty() || ctx.targetBattler.disableTurns > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.disableTurns   = static_cast<int>(Utils::randuint(2, 5));
        ctx.targetBattler.disabledMoveId = ctx.targetBattler.lastMoveId;
        msgFmt(ctx, boost::format(lc::translate("%1%'s move was disabled!")) % ctx.target->getDisplayName());
        ItemUtils::consumeMentalHerb(ctx.target, &ctx.targetBattler, ctx.messages);
        return true;
    case FC::DisableTargetUsingSameMoveConsecutively:
        if (!ctx.target || aromaVeil(ctx))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.torment = 1;
        msgFmt(ctx, boost::format(lc::translate("%1% was subjected to torment!")) % ctx.target->getDisplayName());
        ItemUtils::consumeMentalHerb(ctx.target, &ctx.targetBattler, ctx.messages);
        return true;
    case FC::DisableTargetStatusMoves:
        if (!ctx.target || aromaVeil(ctx)
            || oblivious(ctx))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.tauntTurns = 2;
        msgFmt(ctx, boost::format(lc::translate("%1% fell for the taunt!")) % ctx.target->getDisplayName());
        ItemUtils::consumeMentalHerb(ctx.target, &ctx.targetBattler, ctx.messages);
        return true;

    // Multi-hit shells — effects applied after hits by resolver
    case FC::HitTwoTimes:
    case FC::HitTwoToFiveTimes:
    case FC::HitTwoToFiveTimesOrThreeForAshGreninja:
    case FC::HitTwoToFiveTimesRaiseUserSpd1LowerUserDef1:
    case FC::HitThreeTimesAlwaysCriticalHit:
    case FC::HitThreeTimesPowersUpWithEachHit:
    case FC::HitTwoTimesTargetThenTargetAlly:
    case FC::TwoTurnAttack:
    case FC::TwoTurnAttackBurnTarget:
    case FC::TwoTurnAttackChargeRaiseUserDefense1:
    case FC::TwoTurnAttackChargeRaiseUserSpAtk1:
    case FC::TwoTurnAttackInvulnerableInSky:
    case FC::TwoTurnAttackInvulnerableInSkyParalyzeTarget:
    case FC::TwoTurnAttackInvulnerableInSkyTargetCannotAct:
    case FC::TwoTurnAttackInvulnerableRemoveProtections:
    case FC::TwoTurnAttackInvulnerableUnderground:
    case FC::TwoTurnAttackInvulnerableUnderwater:
    case FC::TwoTurnAttackOneTurnInSun:
    case FC::TwoTurnAttackParalyzeTarget:
    case FC::TwoTurnAttackRaiseUserSpAtkSpDefSpd2:
        if (code == FC::TwoTurnAttackBurnTarget && rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::BURN);
        if ((code == FC::TwoTurnAttackParalyzeTarget
             || code == FC::TwoTurnAttackInvulnerableInSkyParalyzeTarget)
            && rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
        if (code == FC::HitTwoToFiveTimesRaiseUserSpd1LowerUserDef1 && ctx.damageDealt > 0)
        {
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        }
        if (code == FC::TwoTurnAttackRaiseUserSpAtkSpDefSpd2)
        {
            changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 2, false);
            changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 2, false);
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 2, false);
        }
        return true;

    case FC::RaiseUserAttack3IfTargetFaints:
        if (ctx.target->isKO())
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 3, false);
        return true;
    case FC::PowerHigherWithConsecutiveUse:
    case FC::PowerHigherWithConsecutiveUseOnUserSide:
        return true;


    case FC::RaiseTargetAtkSpAtk2:
        // Decorate is ally-only; fails in singles.
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::RaiseTargetRandomStat2:
    {
        auto stat = static_cast<SS>(Utils::randuint(0, static_cast<size_t>(SS::Evasion)));
        changeStage(ctx, ctx.targetBattler, ctx.target, stat, 2, false);
        return true;
    }
    case FC::RaiseUserAtkSpAtk1Or2InSun:
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        return true;
    case FC::RaiseUserMainStats1TrapUserInBattle:
        if (ctx.userBattler.noRetreat)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        ctx.userBattler.trapped   = true;
        ctx.userBattler.noRetreat = true;
        return true;
    case FC::RaiseUserSpDef1PowerUpElectricMove:
        ctx.userBattler.chargeBoost = true;
        msgFmt(ctx, boost::format(lc::translate("%1% began charging power!")) % ctx.user->getDisplayName());
        return true;
    case FC::RaiseAlliesAtkDef1:
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::RaisePlusMinusUserAndAlliesAtkSpAtk1:
    case FC::RaisePlusMinusUserAndAlliesDefSpDef1: {
        bool plusMinus = !ctx.userBattler.abilitySuppressed
                      && (ctx.userBattler.abilityId == "PLUS" || ctx.userBattler.abilityId == "MINUS");
        if (!plusMinus)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        if (ctx.def && ctx.def->getFunctionCode() == FC::RaisePlusMinusUserAndAlliesAtkSpAtk1)
        {
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
            changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialAttack, 1, false);
        }
        else
        {
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
            changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false);
        }
        return true;
    }
    case FC::RaiseGrassBattlersDef1: {
        bool any = false;
        auto buff = [&](Pkmn::PkmnPtr const& mon, BattleBattler& side) {
            if (mon && !mon->isKO() && hasType(mon, "GRASS", &side))
            {
                changeStage(ctx, side, mon, SS::Defense, 1, false);
                any = true;
            }
        };
        buff(ctx.user, ctx.userBattler);
        if (ctx.target && ctx.target != ctx.user)
            buff(ctx.target, ctx.targetBattler);
        if (!any)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        return true;
    }
    case FC::RaiseGroundedGrassBattlersAtkSpAtk1: {
        bool any = false;
        auto buff = [&](Pkmn::PkmnPtr const& mon, BattleBattler& side) {
            if (mon && !mon->isKO() && hasType(mon, "GRASS", &side) && grounded(mon, side, ctx.data))
            {
                changeStage(ctx, side, mon, SS::Attack, 1, false);
                changeStage(ctx, side, mon, SS::SpecialAttack, 1, false);
                any = true;
            }
        };
        buff(ctx.user, ctx.userBattler);
        if (ctx.target && ctx.target != ctx.user)
            buff(ctx.target, ctx.targetBattler);
        if (!any)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        return true;
    }
    case FC::LowerPoisonedTargetAtkSpAtkSpd1:
        if (ctx.target
            && (ctx.target->getStatusCondition() == Pkmn::StatusCondition::POISON
                || ctx.target->getStatusCondition() == Pkmn::StatusCondition::BADLY_POISON))
        {
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true);
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -1, true);
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -1, true);
        }
        return true;
    case FC::LowerTargetSpAtk2IfCanAttract:
        if (!ctx.target || ctx.target->getGender() == Pkmn::Gender::GENDERLESS
            || ctx.user->getGender() == Pkmn::Gender::GENDERLESS
            || ctx.target->getGender() == ctx.user->getGender() || aromaVeil(ctx)
            || oblivious(ctx))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::SpecialAttack, -2, true);
        return true;
    case FC::LowerTargetSpeed1MakeTargetWeakerToFire:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -1, true);
        ctx.targetBattler.tarShot = true;
        if (ctx.target)
            msgFmt(ctx, boost::format(lc::translate("%1% was covered in sticky tar!")) % ctx.target->getDisplayName());
        return true;
    case FC::LowerTargetSpeed1WeakerInGrassyTerrain:
        changeStage(ctx, ctx.targetBattler, ctx.target, SS::Speed, -1, true);
        return true;
    case FC::LowerTargetDefense1PowersUpInGravity:
        if (rollSecondary(ctx) || ctx.def->getCategory() == MoveDef::STATUS)
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Defense, -1, true);
        return true;
    case FC::UserAddStockpileRaiseDefSpDef1:
        if (ctx.userBattler.stockpile < 3)
        {
            ++ctx.userBattler.stockpile;
            int defenseBefore = ctx.userBattler.getStage(SS::Defense);
            if (changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false))
            {
                int gained = ctx.userBattler.getStage(SS::Defense) - defenseBefore;
                if (gained > 0)
                    ctx.userBattler.stockpileDef += gained;
            }
            int specialBefore = ctx.userBattler.getStage(SS::SpecialDefense);
            if (changeStage(ctx, ctx.userBattler, ctx.user, SS::SpecialDefense, 1, false))
            {
                int gained = ctx.userBattler.getStage(SS::SpecialDefense) - specialBefore;
                if (gained > 0)
                    ctx.userBattler.stockpileSpDef += gained;
            }
            msgFmt(ctx, boost::format(lc::translate("%1% stockpiled %2%!")) % ctx.user->getDisplayName()
                            % ctx.userBattler.stockpile);
        }
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    case FC::HealUserDependingOnUserStockpile:
    {
        int s = ctx.userBattler.stockpile;
        if (s <= 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        size_t frac = (s == 1) ? 4 : (s == 2) ? 2 : 1;
        healUser(ctx, maxHpOf(ctx.user) / frac);
        spendStockpile(ctx);
        return true;
    }
    case FC::CurseTargetOrLowerUserSpd1RaiseUserAtkDef1:
        if (hasType(ctx.user, "GHOST", &ctx.userBattler))
        {
            if (!ctx.target || ctx.target == ctx.user || ctx.targetBattler.cursed)
            {
                msg(ctx, lc::translate("But it failed!"));
                return false;
            }
            ctx.user->decreaseHP(std::max<size_t>(1, maxHpOf(ctx.user) / 2));
            ctx.targetBattler.cursed = true;
            msgFmt(ctx, boost::format(lc::translate("%1% cut its own HP and laid a curse on %2%!"))
                            % ctx.user->getDisplayName() % ctx.target->getDisplayName());
            return true;
        }
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, -1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 1, false);
        changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, 1, false);
        return true;
    case FC::MaxUserAttackLoseHalfOfTotalHP: {
        size_t maxHp = maxHpOf(ctx.user);
        if (!ctx.user || maxHp < 2 || ctx.user->getHP() <= maxHp / 2)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        if (!changeStage(ctx, ctx.userBattler, ctx.user, SS::Attack, 12, false))
            return false;
        ctx.user->decreaseHP(std::max<size_t>(1, maxHp / 2));
        msgFmt(ctx, boost::format(lc::translate("%1% cut its own HP and maximized Attack!"))
                        % ctx.user->getDisplayName());
        return true;
    }
    case FC::EnsureNextCriticalHit:
        ctx.userBattler.laserFocus = true;
        msgFmt(ctx, boost::format(lc::translate("%1% took aim for the next turn!")) % ctx.user->getDisplayName());
        return true;
    case FC::HealUserPositionNextTurn:
    {
        int& wish = ctx.playerIsUser ? ctx.data.playerWishTurns : ctx.data.opponentWishTurns;
        size_t& heal = ctx.playerIsUser ? ctx.data.playerWishHeal : ctx.data.opponentWishHeal;
        if (wish > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        wish = 2;
        heal = std::max<size_t>(1, maxHpOf(ctx.user) / 2);
        msgFmt(ctx, boost::format(lc::translate("%1% made a wish!")) % ctx.user->getDisplayName());
        return true;
    }
    case FC::UserStealTargetPositiveStatStages:
        if (ctx.stagesStolen)
            msg(ctx, lc::translate("The stat changes were stolen!"));
        return true;
    case FC::GiveUserStatusToTarget:
        if (ctx.user && ctx.target && ctx.user->getStatusCondition() != Pkmn::StatusCondition::NONE
            && ctx.target->getStatusCondition() == Pkmn::StatusCondition::NONE)
        {
            size_t before = ctx.messages.size();
            if (tryStatus(ctx, ctx.user->getStatusCondition()))
            {
                ctx.user->setStatusCondition(Pkmn::StatusCondition::NONE);
                return true;
            }
            if (ctx.messages.size() == before)
                msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::CureTargetStatusHealUserHalfOfTotalHP:
        if (ctx.target && ctx.target->getStatusCondition() != Pkmn::StatusCondition::NONE)
        {
            ctx.target->setStatusCondition(Pkmn::StatusCondition::NONE);
            healUser(ctx, maxHpOf(ctx.user) / 2);
        }
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    case FC::HealUserDependingOnSandstorm:
    {
        size_t amount = maxHpOf(ctx.user) / 2;
        if (ctx.data.fieldWeather() == Map::SANDSTORM)
            amount = maxHpOf(ctx.user) * 2 / 3;
        healUser(ctx, amount);
        return true;
    }
    case FC::HealUserAndAlliesQuarterOfTotalHP:
    case FC::HealUserAndAlliesQuarterOfTotalHPCureStatus:
        healUser(ctx, maxHpOf(ctx.user) / 4);
        if (code == FC::HealUserAndAlliesQuarterOfTotalHPCureStatus)
            ctx.user->setStatusCondition(Pkmn::StatusCondition::NONE);
        return true;
    case FC::HealUserByTargetAttackLowerTargetAttack1:
    {
        size_t amount = std::max<size_t>(1, ctx.targetBattler.effectiveStat(ctx.target->getStats()[PkmnDef::ATTACK], SS::Attack));
        if (!changeStage(ctx, ctx.targetBattler, ctx.target, SS::Attack, -1, true))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        if (ItemUtils::activeHeldId(ctx.user, &ctx.userBattler) == "BIGROOT")
            amount = std::max<size_t>(1, amount * 13 / 10);
        if (!ctx.targetBattler.abilitySuppressed && ctx.targetBattler.abilityId == "LIQUIDOOZE" && ctx.user)
        {
            if (ctx.userBattler.abilitySuppressed || ctx.userBattler.abilityId != "MAGICGUARD")
            {
                ctx.user->decreaseHP(std::max<size_t>(1, amount));
                msgFmt(ctx, boost::format(lc::translate("%1% was hurt by %2%'s Liquid Ooze!")) % ctx.user->getDisplayName()
                                 % ctx.target->getDisplayName());
            }
        }
        else
            healUser(ctx, amount);
        return true;
    }
    case FC::UserTargetSwapItems:
    {
        auto ui = ctx.user ? ctx.user->getHeldItem() : nullptr;
        auto ti = ctx.target ? ctx.target->getHeldItem() : nullptr;
        if (!ctx.user || !ctx.target || stickyHold(ctx) || (!ui && !ti))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        std::string userLost   = ui && ui->getDefinition() ? ui->getDefinition()->getId() : std::string{};
        std::string targetLost = ti && ti->getDefinition() ? ti->getDefinition()->getId() : std::string{};
        ctx.user->setHeldItem(ti);
        ctx.target->setHeldItem(ui);
        // Trick: Unburden only if that side ends with no item.
        if (ui && !ti)
            ItemUtils::noteHeldItemLoss(ctx.user, &ctx.userBattler, userLost);
        else if (ti)
            ctx.userBattler.unburdenActive = false;
        if (ti && !ui)
            ItemUtils::noteHeldItemLoss(ctx.target, &ctx.targetBattler, targetLost);
        else if (ui)
            ctx.targetBattler.unburdenActive = false;
        msg(ctx, lc::translate("The items were swapped!"));
        return true;
    }
    case FC::UserTakesTargetItem:
        if (ctx.damageDealt > 0 && stickyHold(ctx))
            return true;
        if (ctx.damageDealt > 0 && ctx.target && ctx.target->getHeldItem() && !ctx.user->getHeldItem())
        {
            std::string lost = ItemUtils::activeHeldId(ctx.target, &ctx.targetBattler);
            ctx.user->setHeldItem(ctx.target->getHeldItem());
            ItemUtils::noteHeldItemLoss(ctx.target, &ctx.targetBattler, lost);
            ctx.target->setHeldItem(nullptr);
            ctx.userBattler.unburdenActive = false;
            msg(ctx, lc::translate("The item was stolen!"));
        }
        else
            msg(ctx, lc::translate("But it failed!"));
        return true;
    case FC::UserTargetAverageHP:
    {
        if (ctx.userBattler.healBlockTurns > 0 || ctx.targetBattler.healBlockTurns > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        size_t avg = (ctx.user->getHP() + ctx.target->getHP()) / 2;
        ctx.user->setHP(std::min(avg, maxHpOf(ctx.user)));
        ctx.target->setHP(std::min(avg, maxHpOf(ctx.target)));
        msg(ctx, lc::translate("The battlers shared their HP!"));
        return true;
    }
    case FC::UserTargetSwapBaseSpeed:
        if (!ctx.user || !ctx.target)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        {
            auto currentSpeed = [](Pkmn::PkmnPtr const& mon, BattleBattler const& battler) {
                if (battler.speedOverride >= 0)
                    return battler.speedOverride;
                return static_cast<int>(mon->getStats().at(PkmnDef::SPEED));
            };
            int userSpeed   = currentSpeed(ctx.user, ctx.userBattler);
            int targetSpeed = currentSpeed(ctx.target, ctx.targetBattler);
            ctx.userBattler.speedOverride   = targetSpeed;
            ctx.targetBattler.speedOverride = userSpeed;
            msg(ctx, lc::translate("The battlers swapped their Speed!"));
        }
        return true;
    case FC::FailsIfNotUserFirstTurn:
        if (ctx.userBattler.activeMoveActions > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        return true;
    case FC::DoesNothingCongratulations:
        msg(ctx, lc::translate("Congratulations!"));
        return true;
    case FC::DoesNothingFailsIfNoAlly:
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::DoesNothingUnusableInGravity:
        msg(ctx, lc::translate("But nothing happened!"));
        return true;

    case FC::CounterPhysicalDamage:
    case FC::CounterSpecialDamage:
    case FC::CounterDamagePlusHalf: {
        bool physical = code == FC::CounterPhysicalDamage;
        bool special  = code == FC::CounterSpecialDamage;
        bool categoryOk = code == FC::CounterDamagePlusHalf
                       || (physical && ctx.userBattler.lastDamageCategory == MoveDef::PHYSICAL)
                       || (special && ctx.userBattler.lastDamageCategory == MoveDef::SPECIAL);
        if (!categoryOk || ctx.userBattler.lastDamage <= 0 || !ctx.target)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        size_t dealt = code == FC::CounterDamagePlusHalf
                         ? static_cast<size_t>(ctx.userBattler.lastDamage * 3 / 2)
                         : static_cast<size_t>(ctx.userBattler.lastDamage * 2);
        if (dealt < 1)
            dealt = 1;
        return applyScriptedDamage(ctx, dealt, false);
    }
    case FC::MultiTurnAttackBideThenReturnDoubleDamage:
        if (!ctx.userBattler.bideActive)
        {
            ctx.userBattler.bideActive = true;
            ctx.userBattler.bideTurns  = 1;
            ctx.userBattler.bideDamage = 0;
            msgFmt(ctx, boost::format(lc::translate("%1% is storing energy!")) % ctx.user->getDisplayName());
            return true;
        }
        if (ctx.userBattler.bideTurns > 0)
        {
            --ctx.userBattler.bideTurns;
            msgFmt(ctx, boost::format(lc::translate("%1% is storing energy!")) % ctx.user->getDisplayName());
            return true;
        }
        if (!ctx.target || ctx.userBattler.bideDamage <= 0)
        {
            ctx.userBattler.bideActive = false;
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        {
            size_t dealt = static_cast<size_t>(ctx.userBattler.bideDamage) * 2;
            ctx.target->decreaseHP(std::max<size_t>(1, dealt));
            msgFmt(ctx, boost::format(lc::translate("%1% unleashed energy!")) % ctx.user->getDisplayName());
        }
        ctx.userBattler.bideActive = false;
        ctx.userBattler.bideDamage = 0;
        return true;
    case FC::RemoveProtections:
    case FC::RemoveProtectionsBypassSubstitute:
        ctx.targetBattler.protect     = false;
        ctx.targetBattler.protectKind = BattleBattler::ProtectKind::None;
        if (ctx.playerIsUser)
        {
            ctx.data.opponentWideGuard = ctx.data.opponentQuickGuard = false;
            ctx.data.opponentCraftyShield = ctx.data.opponentMatBlock = false;
        }
        else
        {
            ctx.data.playerWideGuard = ctx.data.playerQuickGuard = false;
            ctx.data.playerCraftyShield = ctx.data.playerMatBlock = false;
        }
        msg(ctx, lc::translate("The protection was broken!"));
        return true;

    case FC::UseRandomMove: {
        auto const& all = Game::instance()->data.moves;
        std::vector<MoveDef::MoveDefPtr> pool;
        for (auto const& candidate : all)
        {
            if (!candidate || candidate->getFunctionCode() == FC::UseRandomMove)
                continue;
            if (candidate->hasFlag(MoveDef::Flag::CannotMetronome))
                continue;
            if (callsAnotherMove(candidate) || isSketch(candidate) || isMimic(candidate) || isBelch(candidate)
                || needsPriorTurnSetup(candidate) || isBideOrUproar(candidate) || isChatter(candidate))
                continue;
            pool.push_back(candidate);
        }
        if (pool.empty())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        msgFmt(ctx, boost::format(lc::translate("%1% used a random move!")) % ctx.user->getDisplayName());
        return callMove(ctx, pool[Utils::randuint(0, pool.size() - 1)]);
    }
    case FC::UseLastMoveUsedByTarget: {
        auto called = moveById(ctx.targetBattler.lastMoveId);
        if (!called || callsAnotherMove(called) || ctx.targetBattler.lastMoveFailed
            || !targetsFoeInSingles(called->getTarget()))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        return callMove(ctx, called);
    }
    case FC::UseLastMoveUsed: {
        auto called = moveById(ctx.data.lastMoveUsedId);
        if (!called || callsAnotherMove(called) || called->hasFlag(MoveDef::Flag::CannotMetronome)
            || needsPriorTurnSetup(called) || isMultiTurnLocked(called) || isBelch(called) || isChatter(called)
            || called->getFunctionCode() == FC::AttackerFaintsIfUserFaints
            || called->getFunctionCode() == FC::RemoveProtections
            || called->getFunctionCode() == FC::PowerUpAllyMove
            || called->getFunctionCode() == FC::CounterPhysicalDamage
            || called->getFunctionCode() == FC::CounterSpecialDamage
            || called->getFunctionCode() == FC::CounterDamagePlusHalf
            || called->getFunctionCode() == FC::FlinchTargetFailsIfNotUserFirstTurn)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        return callMove(ctx, called);
    }
    case FC::UseRandomUserMoveIfAsleep: {
        if (ctx.user->getStatusCondition() != Pkmn::StatusCondition::SLEEP)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        std::vector<Move::MovePtr> pool;
        for (auto const& known : ctx.user->getMoves())
        {
            // Gen 7+: two-turn moves are allowed and finish in one turn.
            if (known && known->getDefinition() && known->getCurrentPP() > 0
                && known->getDefinition()->getFunctionCode() != FC::UseRandomUserMoveIfAsleep
                && !callsAnotherMove(known->getDefinition()) && !isSketch(known->getDefinition())
                && !isBelch(known->getDefinition()) && !isBideOrUproar(known->getDefinition())
                && !isChatter(known->getDefinition()))
                pool.push_back(known);
        }
        if (pool.empty())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        auto picked = pool[Utils::randuint(0, pool.size() - 1)]->getDefinition();
        bool instant = isTwoTurnChargingMove(picked->getFunctionCode());
        return callMove(ctx, picked, instant);
    }
    case FC::UseRandomMoveFromUserParty: {
        std::vector<MoveDef::MoveDefPtr> pool;
        auto consider = [&](auto const& party) {
            for (auto const& member : party)
            {
                if (!member || member == ctx.user)
                    continue;
                for (auto const& known : member->getMoves())
                {
                    if (known && known->getDefinition() && !callsAnotherMove(known->getDefinition())
                        && !isSketch(known->getDefinition()) && !isMimic(known->getDefinition())
                        && !isBelch(known->getDefinition()) && !needsPriorTurnSetup(known->getDefinition())
                        && !isMultiTurnLocked(known->getDefinition()) && !isChatter(known->getDefinition())
                        && !bannedForAssist(known->getDefinition()))
                        pool.push_back(known->getDefinition());
                }
            }
        };
        if (ctx.playerIsUser)
            consider(Game::instance()->data.player.pkmns);
        else if (auto* trainer = dynamic_cast<TrainerSceneData*>(&ctx.data))
        {
            if (trainer->opponentTrainer)
                consider(trainer->opponentTrainer->pkmns);
        }
        if (pool.empty())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        return callMove(ctx, pool[Utils::randuint(0, pool.size() - 1)]);
    }
    case FC::UseMoveTargetIsAboutToUse: {
        Move::MovePtr upcoming = ctx.playerIsUser ? ctx.data.encounterMove : ctx.data.playerMove;
        bool choseMove = ctx.playerIsUser ? (ctx.data.opponentAction == BattleActions::Type::MOVES)
                                          : (ctx.data.playerAction == BattleActions::Type::MOVES);
        bool waiting = choseMove && upcoming && upcoming->getDefinition()
                    && upcoming->getDefinition()->getCategory() != MoveDef::STATUS
                    && !ctx.targetBattler.movedThisTurn && !ctx.targetBattler.switchedInThisTurn;
        if (!waiting
            || movePriority(upcoming->getDefinition(), ctx.target, ctx.targetBattler, ctx.data)
                   > movePriority(ctx.def, ctx.user, ctx.userBattler, ctx.data))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.userBattler.meFirstBoost = true;
        return callMove(ctx, upcoming->getDefinition());
    }
    case FC::TargetUsesItsLastUsedMoveAgain: {
        // Instruct may follow a missed last move; Encore/Mirror still gate on lastMoveFailed.
        if (!ctx.target || ctx.target->isKO() || ctx.targetBattler.lastMoveId.empty())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        Move::MovePtr slot;
        for (auto const& known : ctx.target->getMoves())
        {
            if (known && known->getDefinition() && known->getDefinition()->getId() == ctx.targetBattler.lastMoveId)
            {
                slot = known;
                break;
            }
        }
        auto called = slot ? slot->getDefinition() : moveById(ctx.targetBattler.lastMoveId);
        if (!called || bannedForInstruct(called) || (slot && slot->getCurrentPP() == 0) || ctx.callDepth > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        auto instructed = slot ? slot : std::make_shared<Move>(called);
        auto result = MoveResolver::resolve(ctx.data, !ctx.playerIsUser, instructed, ctx.callDepth + 1);
        ctx.messages.insert(ctx.messages.end(), result.messages.begin(), result.messages.end());
        // Instructed battler is the foe of the Instruct user: flip switch flags.
        ctx.shouldSwitchUser   = result.shouldSwitchTarget;
        ctx.shouldSwitchTarget = result.shouldSwitchUser;
        ctx.forcedSwitchUser   = result.forcedSwitchTarget;
        ctx.forcedSwitchTarget = result.forcedSwitchUser;
        return true;
    }
    case FC::UseMoveDependingOnEnvironment: {
        auto picked = moveById("TRIATTACK");
        if (!picked)
            picked = moveById("TACKLE");
        return callMove(ctx, picked);
    }
    case FC::StealAndUseBeneficialStatusMove:
        (ctx.playerIsUser ? ctx.data.playerSnatch : ctx.data.opponentSnatch) = true;
        msgFmt(ctx, boost::format(lc::translate("%1% waits for a target to make a move!")) % ctx.user->getDisplayName());
        return true;
    case FC::PowerUpAllyMove:
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::AddMoneyGainedFromBattle:
        Game::instance()->data.player.money += ctx.user->getLevel() * 5;
        msg(ctx, lc::translate("Coins scattered everywhere!"));
        return true;
    case FC::DoubleMoneyGainedFromBattle:
        ctx.data.doublePrize = true;
        msg(ctx, lc::translate("The prize money was doubled!"));
        return true;
    case FC::RemoveTargetItem:
        if (ctx.damageDealt == 0 || !ctx.target || !ctx.target->getHeldItem() || stickyHold(ctx))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        {
            std::string lost = ItemUtils::activeHeldId(ctx.target, &ctx.targetBattler);
            ItemUtils::noteHeldItemLoss(ctx.target, &ctx.targetBattler, lost);
            ctx.target->setHeldItem(nullptr);
        }
        refreshPlateType(ctx.target, ctx.targetBattler);
        msgFmt(ctx, boost::format(lc::translate("%1%'s item was knocked away!")) % ctx.target->getDisplayName());
        return true;
    case FC::CorrodeTargetItem:
        if (!ctx.target || !ctx.target->getHeldItem() || stickyHold(ctx))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        {
            std::string lost = ItemUtils::activeHeldId(ctx.target, &ctx.targetBattler);
            ItemUtils::noteHeldItemLoss(ctx.target, &ctx.targetBattler, lost);
            ctx.target->setHeldItem(nullptr);
        }
        refreshPlateType(ctx.target, ctx.targetBattler);
        msgFmt(ctx, boost::format(lc::translate("%1%'s item was corroded!")) % ctx.target->getDisplayName());
        return true;
    case FC::DestroyTargetBerryOrGem: {
        if (ctx.damageDealt == 0 || !ctx.target || !ctx.target->getHeldItem() || !ctx.target->getHeldItem()->getDefinition())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        std::string id = ctx.target->getHeldItem()->getDefinition()->getId();
        if (!berryOrGem(id))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ItemUtils::noteHeldItemLoss(ctx.target, &ctx.targetBattler, id);
        ctx.target->setHeldItem(nullptr);
        refreshPlateType(ctx.target, ctx.targetBattler);
        msgFmt(ctx, boost::format(lc::translate("%1%'s %2% was burned up!")) % ctx.target->getDisplayName() % id);
        return true;
    }
    case FC::ThrowUserItemAtTarget: {
        auto held = ctx.user ? ctx.user->getHeldItem() : nullptr;
        if (ctx.damageDealt == 0 || !held || !held->getDefinition() || !ctx.target)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        std::string thrown = held->getDefinition()->getId();
        ctx.userBattler.recycledItemId = thrown;
        ItemUtils::noteHeldItemLoss(ctx.user, &ctx.userBattler, thrown);
        ctx.user->setHeldItem(nullptr);
        refreshPlateType(ctx.user, ctx.userBattler);
        if (thrown == "POISONBARB")
            tryStatus(ctx, Pkmn::StatusCondition::POISON);
        else if (thrown == "FLAMEORB")
            tryStatus(ctx, Pkmn::StatusCondition::BURN);
        else if (thrown == "LIGHTBALL" || thrown == "KINGSROCK")
            tryFlinch(ctx);
        msgFmt(ctx, boost::format(lc::translate("%1% flung its item!")) % ctx.user->getDisplayName());
        return true;
    }
    case FC::UserConsumeTargetBerry: {
        if (ctx.damageDealt == 0 || !ctx.target || !ctx.user || !ctx.target->getHeldItem()
            || !ctx.target->getHeldItem()->getDefinition()
            || ctx.target->getHeldItem()->getDefinition()->getId().find("BERRY") == std::string::npos)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        auto berry = ctx.target->getHeldItem();
        ItemUtils::noteHeldItemLoss(ctx.target, &ctx.targetBattler, berry->getDefinition()->getId());
        ctx.target->setHeldItem(nullptr);
        refreshPlateType(ctx.target, ctx.targetBattler);
        ConfusionStatus* confusion = ctx.playerIsUser ? &ctx.data.playerConfusion : &ctx.data.opponentConfusion;
        auto eaten = ItemUtils::eatBerry(ctx.user, &ctx.userBattler, confusion, berry, ctx.data);
        ctx.userBattler.consumedBerry = true;
        if (eaten.ejected)
            ctx.shouldSwitchUser = true;
        if (!eaten.message.empty())
            msg(ctx, eaten.message);
        else
            msgFmt(ctx, boost::format(lc::translate("%1% ate the foe's berry!")) % ctx.user->getDisplayName());
        return true;
    }
    case FC::UserConsumeBerryRaiseDefense2: {
        std::string held = ItemUtils::activeHeldId(ctx.user, &ctx.userBattler);
        if (held.find("BERRY") == std::string::npos)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ConfusionStatus* confusion = ctx.playerIsUser ? &ctx.data.playerConfusion : &ctx.data.opponentConfusion;
        eatHeldBerry(ctx, ctx.user, ctx.userBattler, confusion);
        auto raised = applyStatChange(ctx.data, ctx.userBattler, ctx.user, SS::Defense, 2, ctx.messages, false);
        if (raised.raised)
        {
            ctx.statRose = true;
            StatAnimNote note;
            note.palette  = statAnimPalette(BattleBattler::StageStat::Defense);
            note.rose     = true;
            note.onTarget = false;
            note.sharp    = std::abs(raised.applied) >= 2;
            ctx.statNotes.push_back(note);
        }
        if (raised.ejected)
            ctx.shouldSwitchUser = true;
        return true;
    }
    case FC::RestoreUserConsumedItem: {
        if (ctx.userBattler.recycledItemId.empty() || ctx.user->getHeldItem())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        auto def = Game::instance()->data.itemDefFor(ctx.userBattler.recycledItemId);
        if (!def)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.user->setHeldItem(std::make_shared<Item>(def, 1));
        ctx.userBattler.recycledItemId.clear();
        msgFmt(ctx, boost::format(lc::translate("%1% found its item again!")) % ctx.user->getDisplayName());
        return true;
    }

    case FC::AddGhostTypeToTarget:
    case FC::AddGrassTypeToTarget: {
        std::string added = code == FC::AddGhostTypeToTarget ? "GHOST" : "GRASS";
        auto types = activeTypes(ctx.target, &ctx.targetBattler);
        if (std::find(types.begin(), types.end(), added) == types.end())
            types.push_back(added);
        ctx.targetBattler.typesOverride = types;
        msgFmt(ctx, boost::format(lc::translate("%1% gained a type!")) % ctx.target->getDisplayName());
        return true;
    }
    case FC::SetTargetTypesToWater:
        ctx.targetBattler.typesOverride = {"WATER"};
        msgFmt(ctx, boost::format(lc::translate("%1% transformed into the Water type!")) % ctx.target->getDisplayName());
        return true;
    case FC::SetTargetTypesToPsychic:
        ctx.targetBattler.typesOverride = {"PSYCHIC"};
        msgFmt(ctx, boost::format(lc::translate("%1% transformed into the Psychic type!")) % ctx.target->getDisplayName());
        return true;
    case FC::SetUserTypesToTargetTypes:
        ctx.userBattler.typesOverride = activeTypes(ctx.target, &ctx.targetBattler);
        msgFmt(ctx, boost::format(lc::translate("%1% copied the foe's type!")) % ctx.user->getDisplayName());
        return true;
    case FC::SetUserTypesToUserMoveType: {
        std::string type = ctx.def->getType();
        for (auto const& known : ctx.user->getMoves())
        {
            if (known && known->getDefinition() && known->getDefinition()->getFunctionCode() != code)
            {
                type = known->getDefinition()->getType();
                break;
            }
        }
        ctx.userBattler.typesOverride = {type};
        msgFmt(ctx, boost::format(lc::translate("%1%'s type changed!")) % ctx.user->getDisplayName());
        return true;
    }
    case FC::SetUserTypesToResistLastAttack: {
        if (ctx.userBattler.lastHitType.empty())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        std::vector<std::string> options;
        for (auto const& type : Game::instance()->data.types)
        {
            if (!type)
                continue;
            auto const& resist = type->getResistances();
            auto const& immune = type->getImmunities();
            if (std::find(resist.begin(), resist.end(), ctx.userBattler.lastHitType) != resist.end()
                || std::find(immune.begin(), immune.end(), ctx.userBattler.lastHitType) != immune.end())
                options.push_back(type->getId());
        }
        if (options.empty())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.userBattler.typesOverride = {options[Utils::randuint(0, options.size() - 1)]};
        msgFmt(ctx, boost::format(lc::translate("%1%'s type changed!")) % ctx.user->getDisplayName());
        return true;
    }
    case FC::SetUserTypesBasedOnEnvironment:
        ctx.userBattler.typesOverride = {"NORMAL"};
        msgFmt(ctx, boost::format(lc::translate("%1%'s type changed!")) % ctx.user->getDisplayName());
        return true;
    case FC::UserLosesFireType: {
        if (ctx.damageDealt == 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        auto types = activeTypes(ctx.user, &ctx.userBattler);
        types.erase(std::remove(types.begin(), types.end(), "FIRE"), types.end());
        if (types.empty())
            types.push_back("NORMAL");
        ctx.userBattler.typesOverride = types;
        msgFmt(ctx, boost::format(lc::translate("%1% burned away its Fire type!")) % ctx.user->getDisplayName());
        return true;
    }
    case FC::NormalMovesBecomeElectric:
        ctx.data.ionDeluge = true;
        msg(ctx, lc::translate("A deluge of ions showered the battlefield!"));
        return true;
    case FC::TargetMovesBecomeElectric:
        ctx.targetBattler.movesBecomeElectric = true;
        msgFmt(ctx, boost::format(lc::translate("%1%'s moves have been electrified!")) % ctx.target->getDisplayName());
        return true;
    case FC::StartNegateHeldItems:
    case FC::StartSwapAllBattlersBaseDefensiveStats:
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::NegateTargetAbility:
        ctx.targetBattler.abilitySuppressed = true;
        msgFmt(ctx, boost::format(lc::translate("%1%'s ability was suppressed!")) % ctx.target->getDisplayName());
        return true;
    case FC::NegateTargetAbilityIfTargetActed:
        if (!ctx.target || !ctx.targetBattler.movedThisTurn || ctx.damageDealt == 0)
            return true;
        ctx.targetBattler.abilitySuppressed = true;
        msgFmt(ctx, boost::format(lc::translate("%1%'s ability was suppressed!")) % ctx.target->getDisplayName());
        return true;
    case FC::SetTargetAbilityToInsomnia:
        ctx.targetBattler.abilityId = "INSOMNIA";
        ctx.targetBattler.abilitySuppressed = false;
        msgFmt(ctx, boost::format(lc::translate("%1%'s ability became Insomnia!")) % ctx.target->getDisplayName());
        return true;
    case FC::SetTargetAbilityToSimple:
        ctx.targetBattler.abilityId = "SIMPLE";
        ctx.targetBattler.abilitySuppressed = false;
        msgFmt(ctx, boost::format(lc::translate("%1%'s ability became Simple!")) % ctx.target->getDisplayName());
        return true;
    case FC::SetTargetAbilityToUserAbility:
        ctx.targetBattler.abilityId = ctx.userBattler.abilityId;
        ctx.targetBattler.abilitySuppressed = false;
        msgFmt(ctx, boost::format(lc::translate("%1% copied %2%'s ability!")) % ctx.target->getDisplayName()
                         % ctx.user->getDisplayName());
        return true;
    case FC::SetUserAbilityToTargetAbility:
        ctx.userBattler.abilityId = ctx.targetBattler.abilityId;
        ctx.userBattler.abilitySuppressed = false;
        msgFmt(ctx, boost::format(lc::translate("%1% copied %2%'s ability!")) % ctx.user->getDisplayName()
                         % ctx.target->getDisplayName());
        return true;
    case FC::UserTargetSwapAbilities:
        std::swap(ctx.userBattler.abilityId, ctx.targetBattler.abilityId);
        msg(ctx, lc::translate("The battlers swapped abilities!"));
        return true;
    case FC::EffectDependsOnEnvironment:
        if (!rollSecondary(ctx))
            return true;
        tryStatus(ctx, Pkmn::StatusCondition::PARALYSIS);
        return true;
    case FC::HealTargetDependingOnGrassyTerrain:
        if (!ctx.target || ctx.targetBattler.healBlockTurns > 0)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        {
            size_t heal = std::max<size_t>(1, ctx.target->getTotalHP() / 2);
            ctx.target->setHP(std::min(ctx.target->getTotalHP(), ctx.target->getHP() + heal));
            msgFmt(ctx, boost::format(lc::translate("%1% had its HP restored.")) % ctx.target->getDisplayName());
        }
        return true;
    case FC::DisableTargetHealingMoves:
        if (!ctx.target || aromaVeil(ctx))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.healBlockTurns = 5;
        msgFmt(ctx, boost::format(lc::translate("%1% was prevented from healing!")) % ctx.target->getDisplayName());
        ItemUtils::consumeMentalHerb(ctx.target, &ctx.targetBattler, ctx.messages);
        return true;
    case FC::DisableTargetSoundMoves:
        if (ctx.damageDealt == 0)
            return true;
        ctx.targetBattler.soundBlockTurns = 2;
        msgFmt(ctx, boost::format(lc::translate("%1% was prevented from using sound!")) % ctx.target->getDisplayName());
        return true;
    case FC::DisableTargetUsingDifferentMove:
        if (!ctx.target || aromaVeil(ctx) || ctx.targetBattler.lastMoveId.empty() || ctx.targetBattler.lastMoveId == "ENCORE"
            || ctx.targetBattler.encoreTurns > 0 || ctx.targetBattler.lastMoveFailed)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.encoreTurns  = static_cast<int>(Utils::randuint(3, 6));
        ctx.targetBattler.encoreMoveId = ctx.targetBattler.lastMoveId;
        msgFmt(ctx, boost::format(lc::translate("%1% received an encore!")) % ctx.target->getDisplayName());
        ItemUtils::consumeMentalHerb(ctx.target, &ctx.targetBattler, ctx.messages);
        return true;
    case FC::DisableTargetMovesKnownByUser:
        ctx.userBattler.imprison = true;
        msg(ctx, lc::translate("Imprison seals the shared moves!"));
        return true;
    case FC::LowerPPOfTargetLastMoveBy3:
    case FC::LowerPPOfTargetLastMoveBy4: {
        int loss = code == FC::LowerPPOfTargetLastMoveBy4 ? static_cast<int>(Utils::randuint(2, 5)) : 3;
        bool found = false;
        if (ctx.target)
        {
            for (auto& slot : ctx.target->getMoves())
            {
                if (slot && slot->getDefinition() && slot->getDefinition()->getId() == ctx.targetBattler.lastMoveId
                    && slot->getCurrentPP() > 0)
                {
                    for (int i = 0; i < loss && slot->getCurrentPP() > 0; ++i)
                        slot->decreasePP();
                    found = true;
                }
            }
        }
        if (!found)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        msgFmt(ctx, boost::format(lc::translate("%1%'s PP was reduced!")) % ctx.target->getDisplayName());
        return true;
    }
    case FC::ReplaceMoveWithTargetLastMoveUsed:
    case FC::ReplaceMoveThisBattleWithTargetLastMoveUsed: {
        if (ctx.targetBattler.lastMoveId.empty())
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        auto copied = Game::instance()->data.moveDefFor(ctx.targetBattler.lastMoveId);
        if (!copied)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        std::string copiedId = copied->getId();
        if (copiedId == "STRUGGLE" || copiedId == "CHATTER" || copiedId == "SKETCH" || copiedId == "TRANSFORM")
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        auto& slots = ctx.user->getMoves();
        for (int i = 0; i < 4; ++i)
        {
            if (slots[i] && slots[i]->getDefinition() && slots[i]->getDefinition()->getId() == copiedId)
            {
                msg(ctx, lc::translate("But it failed!"));
                return false;
            }
        }
        for (int i = 0; i < 4; ++i)
        {
            if (slots[i] && slots[i]->getDefinition() && slots[i]->getDefinition()->getId() == ctx.def->getId())
            {
                bool mimic = code == FC::ReplaceMoveThisBattleWithTargetLastMoveUsed;
                if (mimic)
                {
                    ctx.userBattler.mimicActive     = true;
                    ctx.userBattler.mimicSlot       = i;
                    ctx.userBattler.mimicOriginalId = ctx.def->getId();
                    ctx.userBattler.mimicOriginalPP = slots[i]->getCurrentPP();
                }
                int pp = static_cast<int>(copied->getTotalPP());
                if (mimic)
                    pp = std::min(5, pp);
                slots[i] = std::make_shared<Move>(copied);
                slots[i]->setCurrentPP(pp);
                msgFmt(ctx, boost::format(lc::translate("%1% learned %2%!")) % ctx.user->getDisplayName() % copied->getName());
                return true;
            }
        }
        msg(ctx, lc::translate("But it failed!"));
        return false;
    }
    case FC::TargetTakesUserItem:
        if (!ctx.user->getHeldItem() || (ctx.target && ctx.target->getHeldItem()))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        {
            std::string lost = ItemUtils::activeHeldId(ctx.user, &ctx.userBattler);
            ctx.target->setHeldItem(ctx.user->getHeldItem());
            ItemUtils::noteHeldItemLoss(ctx.user, &ctx.userBattler, lost);
            ctx.user->setHeldItem(nullptr);
            ctx.targetBattler.unburdenActive = false;
        }
        msgFmt(ctx, boost::format(lc::translate("%1% received the item!")) % ctx.target->getDisplayName());
        return true;
    case FC::StartTargetCannotUseItem:
        ctx.targetBattler.embargoTurns = 5;
        msgFmt(ctx, boost::format(lc::translate("%1% can't use items!")) % ctx.target->getDisplayName());
        return true;
    case FC::AllBattlersConsumeBerry: {
        eatHeldBerry(ctx, ctx.data.playerPkmn, ctx.data.playerBattler, &ctx.data.playerConfusion);
        eatHeldBerry(ctx, ctx.data.opponentPkmn, ctx.data.opponentBattler, &ctx.data.opponentConfusion);
        msg(ctx, lc::translate("The battlers ate their berries!"));
        return true;
    }
    case FC::StartUserSideDoubleSpeed:
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::StartPreventCriticalHitsAgainstUserSide:
        if (ctx.playerIsUser)
            ctx.data.playerLuckyChant = 5;
        else
            ctx.data.opponentLuckyChant = 5;
        msg(ctx, lc::translate("Lucky Chant shields the team from critical hits!"));
        return true;
    case FC::StartWeakenElectricMoves:
        ctx.data.mudSportTurns = 5;
        msg(ctx, lc::translate("Electricity's power was weakened!"));
        return true;
    case FC::StartWeakenFireMoves:
        ctx.data.waterSportTurns = 5;
        msg(ctx, lc::translate("Fire's power was weakened!"));
        return true;
    case FC::StartUserAirborne:
        ctx.userBattler.airborneTurns = 5;
        msgFmt(ctx, boost::format(lc::translate("%1% levitated with Magnet Rise!")) % ctx.user->getDisplayName());
        return true;
    case FC::StartTargetAirborneAndAlwaysHitByMoves:
        ctx.targetBattler.airborneTurns = 3;
        ctx.targetBattler.telekinesis = true;
        msgFmt(ctx, boost::format(lc::translate("%1% was hurled into the air!")) % ctx.target->getDisplayName());
        return true;
    case FC::StartDamageTargetEachTurnIfTargetAsleep:
        if (!ctx.target || ctx.target->getStatusCondition() != Pkmn::StatusCondition::SLEEP)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.nightmare = true;
        msgFmt(ctx, boost::format(lc::translate("%1% fell into a nightmare!")) % ctx.target->getDisplayName());
        return true;
    case FC::StartHealUserEachTurnTrapUserInBattle:
        ctx.userBattler.ingrain = true;
        ctx.userBattler.trapped = true;
        msgFmt(ctx, boost::format(lc::translate("%1% planted its roots!")) % ctx.user->getDisplayName());
        return true;
    case FC::TrapTargetInBattleLowerTargetDefSpDef1EachTurn:
        if (ctx.damageDealt == 0 || ctx.hitSubstitute)
            return true;
        ctx.targetBattler.octolock = true;
        ctx.targetBattler.trapped = true;
        msgFmt(ctx, boost::format(lc::translate("%1% was locked in!")) % ctx.target->getDisplayName());
        return true;
    case FC::TrapUserAndTargetInBattle:
        if (ctx.damageDealt == 0 || ctx.hitSubstitute)
            return true;
        ctx.userBattler.trapped = true;
        ctx.userBattler.jawLocked = true;
        ctx.targetBattler.trapped = true;
        ctx.targetBattler.jawLocked = true;
        msg(ctx, lc::translate("Neither Pokémon can flee!"));
        return true;
    case FC::TrapAllBattlersInBattleForOneTurn:
        ctx.data.fairyLock      = true;
        ctx.data.fairyLockTurns = 2;
        msg(ctx, lc::translate("Nobody can flee next turn!"));
        return true;
    case FC::AttackerFaintsIfUserFaints:
        if (ctx.userBattler.destinyBondLock)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.userBattler.destinyBond = true;
        msgFmt(ctx, boost::format(lc::translate("%1% is trying to take its foe with it!")) % ctx.user->getDisplayName());
        return true;
    case FC::UserFaintsHealAndCureReplacement:
    case FC::UserFaintsHealAndCureReplacementRestorePP:
        if (!sideHasReplacement(ctx, ctx.playerIsUser))
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.user->decreaseHP(ctx.user->getHP());
        if (ctx.playerIsUser)
        {
            ctx.data.playerHealingWish = true;
            ctx.data.playerLunarDance = code == FC::UserFaintsHealAndCureReplacementRestorePP;
        }
        else
        {
            ctx.data.opponentHealingWish = true;
            ctx.data.opponentLunarDance = code == FC::UserFaintsHealAndCureReplacementRestorePP;
        }
        msgFmt(ctx, boost::format(lc::translate("%1% wished for a healthy replacement!")) % ctx.user->getDisplayName());
        return true;
    case FC::BounceBackProblemCausingStatusMoves:
        ctx.userBattler.magicCoat = true;
        msgFmt(ctx, boost::format(lc::translate("%1% was covered by a Magic Coat!")) % ctx.user->getDisplayName());
        return true;
    case FC::SwapSideEffects:
        std::swap(ctx.data.playerReflect, ctx.data.opponentReflect);
        std::swap(ctx.data.playerLightScreen, ctx.data.opponentLightScreen);
        std::swap(ctx.data.playerAuroraVeil, ctx.data.opponentAuroraVeil);
        std::swap(ctx.data.playerSafeguard, ctx.data.opponentSafeguard);
        std::swap(ctx.data.playerMist, ctx.data.opponentMist);
        std::swap(ctx.data.playerSpikes, ctx.data.opponentSpikes);
        std::swap(ctx.data.playerLuckyChant, ctx.data.opponentLuckyChant);
        msg(ctx, lc::translate("The two sides switched effects!"));
        return true;
    case FC::UserTargetAverageBaseAtkSpAtk: {
        size_t atk = (ctx.user->getStats()[PkmnDef::ATTACK] + ctx.target->getStats()[PkmnDef::ATTACK]) / 2;
        size_t spa = (ctx.user->getStats()[PkmnDef::SPECIAL_ATTACK] + ctx.target->getStats()[PkmnDef::SPECIAL_ATTACK]) / 2;
        ctx.userBattler.attackOverride = static_cast<int>(atk);
        ctx.targetBattler.attackOverride = static_cast<int>(atk);
        ctx.userBattler.spAttackOverride = static_cast<int>(spa);
        ctx.targetBattler.spAttackOverride = static_cast<int>(spa);
        msg(ctx, lc::translate("The battlers shared their attacking stats!"));
        return true;
    }
    case FC::UserTargetAverageBaseDefSpDef: {
        size_t def = (ctx.user->getStats()[PkmnDef::DEFENSE] + ctx.target->getStats()[PkmnDef::DEFENSE]) / 2;
        size_t spd = (ctx.user->getStats()[PkmnDef::SPECIAL_DEFENSE] + ctx.target->getStats()[PkmnDef::SPECIAL_DEFENSE]) / 2;
        ctx.userBattler.defenseOverride = static_cast<int>(def);
        ctx.targetBattler.defenseOverride = static_cast<int>(def);
        ctx.userBattler.spDefenseOverride = static_cast<int>(spd);
        ctx.targetBattler.spDefenseOverride = static_cast<int>(spd);
        msg(ctx, lc::translate("The battlers shared their defensive stats!"));
        return true;
    }
    case FC::StartRaiseUserAtk1WhenDamaged:
        ctx.userBattler.rage = true;
        msgFmt(ctx, boost::format(lc::translate("%1%'s rage is building!")) % ctx.user->getDisplayName());
        return true;
    case FC::StartNegateTargetEvasionStatStageAndGhostImmunity:
        if (ctx.targetBattler.negateGhostImmunity)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.ignoreEvasion = true;
        ctx.targetBattler.negateGhostImmunity = true;
        if (int evasion = ctx.targetBattler.getStage(SS::Evasion); evasion != 0)
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Evasion, -evasion, true);
        msgFmt(ctx, boost::format(lc::translate("%1% was identified!")) % ctx.target->getDisplayName());
        return true;
    case FC::StartNegateTargetEvasionStatStageAndDarkImmunity:
        if (ctx.targetBattler.negateDarkImmunity)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        ctx.targetBattler.ignoreEvasion = true;
        ctx.targetBattler.negateDarkImmunity = true;
        if (int evasion = ctx.targetBattler.getStage(SS::Evasion); evasion != 0)
            changeStage(ctx, ctx.targetBattler, ctx.target, SS::Evasion, -evasion, true);
        msgFmt(ctx, boost::format(lc::translate("%1% was identified!")) % ctx.target->getDisplayName());
        return true;
    case FC::EnsureNextMoveAlwaysHits:
        ctx.userBattler.lockOn = true;
        msgFmt(ctx, boost::format(lc::translate("%1% took aim!")) % ctx.user->getDisplayName());
        return true;
    case FC::BurnAttackerBeforeUserActs:
        // Armed and announced at command lock; contact burn is applied on hit elsewhere.
        return true;
    case FC::AttackTwoTurnsLater: {
        DelayedStrike& slot = ctx.playerIsUser ? ctx.data.playerDelayed : ctx.data.opponentDelayed;
        if (slot.turns > 0 || !ctx.user || !ctx.target)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        slot.turns        = 3;
        slot.hitsPlayer   = !ctx.playerIsUser;
        slot.moveId       = ctx.def->getId();
        slot.user         = ctx.user;
        slot.userIsPlayer = ctx.playerIsUser;
        float screen = 1.f;
        int   light  = slot.hitsPlayer ? ctx.data.playerLightScreen : ctx.data.opponentLightScreen;
        if (light > 0)
            screen = 0.5f;
        DamageAdjustments delayedAdjust;
        delayedAdjust.forceSpecial = true;
        auto delayed = SingleBattleScene::computeDamage(ctx.user, ctx.target, ctx.move, ctx.data.fieldWeather(),
                                                        &ctx.userBattler, &ctx.targetBattler, ctx.def->getPower(), false,
                                                        false, screen, false, "TYPELESS", delayedAdjust);
        slot.damage = std::max<size_t>(1, delayed.amount);
        msg(ctx, lc::translate("It chose a move that will strike later!"));
        return true;
    }
    case FC::HitsTargetInSkyGroundsTarget:
        if (ctx.damageDealt == 0 || ctx.hitSubstitute)
            return false;
        setHide(ctx.targetBattler, BattleBattler::SemiKind::None);
        ctx.targetBattler.airborneTurns = 0;
        ctx.targetBattler.groundedByMove = true;
        msgFmt(ctx, boost::format(lc::translate("%1% was knocked to the ground!")) % ctx.target->getDisplayName());
        return true;
    case FC::TargetActsLast: {
        if (!ctx.target || ctx.targetBattler.movedThisTurn)
        {
            msg(ctx, lc::translate("But it failed!"));
            return false;
        }
        if (ctx.playerIsUser)
            ctx.data.opponentMustMoveLast = true;
        else
            ctx.data.playerMustMoveLast = true;
        msgFmt(ctx, boost::format(lc::translate("%1% will move last!")) % ctx.target->getDisplayName());
        return true;
    }
    case FC::TargetActsNext:
        // After You requires an ally — always fails in singles.
        msg(ctx, lc::translate("But it failed!"));
        return false;
    case FC::SetAttackerMovePPTo0IfUserFaints:
        ctx.userBattler.grudge = true;
        msgFmt(ctx, boost::format(lc::translate("%1% wants its foe to bear a grudge!")) % ctx.user->getDisplayName());
        return true;
    case FC::TargetNextFireMoveDamagesTarget:
        ctx.targetBattler.powder = true;
        msgFmt(ctx, boost::format(lc::translate("%1% was covered in powder!")) % ctx.target->getDisplayName());
        return true;
    case FC::CategoryDependsOnHigherDamagePoisonTarget:
        if (ctx.damageDealt > 0 && rollSecondary(ctx))
            tryStatus(ctx, Pkmn::StatusCondition::POISON);
        return true;
    // Remaining Essentials FunctionCodes: Gen III best-effort / documented no-op
    case FC::CannotBeRedirected:
    case FC::CannotMakeTargetFaint:
    case FC::DamageTargetAlly:
    case FC::EffectivenessIncludesFlyingType:
    case FC::FailsIfTargetActed:
    case FC::FailsIfTargetHasNoItem:
    case FC::FailsIfUserDamagedThisTurn:
    case FC::FailsIfUserHasUnusedMove:
    case FC::FailsIfUserNotConsumedBerry:
    case FC::FailsUnlessTargetSharesTypeWithUser:
    case FC::HealAllyOrDamageFoe:
    case FC::HigherPriorityInGrassyTerrain:
    case FC::HitsAllFoesAndPowersUpInPsychicTerrain:
    case FC::HitsTargetInSky:
        return true;
    case FC::HoopaRemoveProtectionsBypassSubstituteLowerUserDef1:
        if (ctx.damageDealt > 0)
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Defense, -1, false);
        return true;
    case FC::TypeDependsOnUserMorpekoFormRaiseUserSpeed1:
        if (ctx.damageDealt > 0)
            changeStage(ctx, ctx.userBattler, ctx.user, SS::Speed, 1, false);
        return true;
    case FC::IgnoreTargetAbility:
    case FC::IgnoreTargetDefSpDefEvaStatStages:
    case FC::PowerHigherWithTargetWeight:
    case FC::PowerHigherWithUserHappiness:
    case FC::PowerHigherWithUserHeavierThanTarget:
    case FC::PowerLowerWithUserHappiness:
    case FC::PursueSwitchingFoe:
    case FC::RandomlyDamageOrHealTarget:
    case FC::TypeAndPowerDependOnTerrain:
    case FC::TypeAndPowerDependOnUserBerry:
    case FC::TypeAndPowerDependOnWeather:
    case FC::TypeDependsOnUserDrive:
    case FC::TypeDependsOnUserIVs:
    case FC::TypeDependsOnUserMemory:
    case FC::TypeDependsOnUserPlate:
    case FC::TypeIsUserFirstType:
    case FC::UseTargetAttackInsteadOfUserAttack:
    case FC::UseTargetDefenseInsteadOfTargetSpDef:
    case FC::UseUserDefenseInsteadOfUserAttack:
    case FC::UsedAfterAllyRoundWithDoublePower:
    case FC::UsedAfterUserTakesPhysicalDamage:
        return true; // power / fail gates handled elsewhere
    case FC::RedirectAllMovesToTarget:
    case FC::RedirectAllMovesToUser:
    case FC::UserSwapsPositionsWithAlly:
        // Follow Me / Rage Powder / Spotlight / Ally Switch — doubles-only; fail in singles.
        msg(ctx, lc::translate("But it failed!"));
        return false;


    case FC::HealUserFullyAndFallAsleep:
        // handled above; keep for exhaustiveness
        return true;
    case FC::AlwaysCriticalHit:
    case FC::DoublePowerAfterFusionBolt:
    case FC::DoublePowerAfterFusionFlare:
    case FC::DoublePowerIfAllyFaintedLastTurn:
    case FC::DoublePowerIfTargetActed:
    case FC::DoublePowerIfTargetHPLessThanHalf:
    case FC::DoublePowerIfTargetInSky:
    case FC::DoublePowerIfTargetLostHPThisTurn:
    case FC::DoublePowerIfTargetNotActed:
    case FC::DoublePowerIfTargetPoisoned:
    case FC::DoublePowerIfTargetStatusProblem:
    case FC::DoublePowerIfTargetUnderground:
    case FC::DoublePowerIfTargetUnderwater:
    case FC::DoublePowerIfUserHasNoItem:
    case FC::DoublePowerIfUserLastMoveFailed:
    case FC::DoublePowerIfUserLostHPThisTurn:
    case FC::DoublePowerIfUserPoisonedBurnedParalyzed:
    case FC::DoublePowerIfUserStatsLoweredThisTurn:
    case FC::DoublePowerInElectricTerrain:
    case FC::PowerHigherWithLessPP:
    case FC::PowerHigherWithTargetFasterThanUser:
    case FC::PowerHigherWithTargetHP:
    case FC::PowerHigherWithTargetPositiveStatStages:
    case FC::PowerHigherWithUserFasterThanTarget:
    case FC::PowerHigherWithUserHP:
    case FC::PowerHigherWithUserPositiveStatStages:
    case FC::PowerLowerWithUserHP:
    case FC::RandomPowerDoublePowerIfTargetUnderground:
        return true; // power handled in moveEffectPower
    case FC::PowerDependsOnUserStockpile:
        return true;
    case FC::DoublePowerIfTargetAsleepCureTarget:
        if (ctx.target && ctx.target->getStatusCondition() == Pkmn::StatusCondition::SLEEP)
        {
            ctx.target->setStatusCondition(Pkmn::StatusCondition::NONE);
            msgFmt(ctx, boost::format(lc::translate("%1% woke up!")) % ctx.target->getDisplayName());
        }
        return true;
    case FC::DoublePowerIfTargetParalyzedCureTarget:
        if (ctx.target && ctx.target->getStatusCondition() == Pkmn::StatusCondition::PARALYSIS)
        {
            ctx.target->setStatusCondition(Pkmn::StatusCondition::NONE);
            msgFmt(ctx, boost::format(lc::translate("%1% was cured of paralysis!")) % ctx.target->getDisplayName());
        }
        return true;

    default:
        // Explicit documented no-op / best-effort for remaining Essentials codes
        // (abilities, complex item/type interactions, pledges, etc.).
        return true;
    }
}
