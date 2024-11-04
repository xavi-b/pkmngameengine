#ifndef PKMN_H
#define PKMN_H

#include "move.h"
#include "pkmndef.h"

#include <array>
#include <vector>

class Pkmn
{
public:
    enum class StatusCondition : size_t
    {
        NONE,
        BURN,
        FREEZE,
        PARALYSIS,
        POISON,
        BADLY_POISON,
        SLEEP
    };

    enum class Gender : size_t
    {
        MALE,
        FEMALE,
        GENDERLESS
    };

    static constexpr unsigned char HighFriendship = 160;

    using PkmnPtr = std::shared_ptr<Pkmn>;

    friend void    tag_invoke(js::value_from_tag, js::value& jv, PkmnPtr const& o);
    friend PkmnPtr tag_invoke(js::value_to_tag<PkmnPtr>, js::value const& jv);

    Pkmn(PkmnDef::PkmnDefPtr definition, size_t level);

    std::map<PkmnDef::Stat, size_t> getStats() const;
    size_t                          getTotalHP() const;

    std::map<PkmnDef::Stat, size_t> getIVs() const;
    void                            setIVs(std::map<PkmnDef::Stat, size_t> const& newIVs);

    std::map<PkmnDef::Stat, size_t> getEVs() const;
    void                            setEVs(std::map<PkmnDef::Stat, size_t> const& newEVs);

    std::array<Move::MovePtr, 4> const& getMoves() const;
    std::array<Move::MovePtr, 4>&       getMoves();
    void                                setMoves(std::array<Move::MovePtr, 4> const& newMoves);
    void                                generateFromPkmnDef();
    void                                addMovesFromPkmnDef();

    size_t getLevel() const;
    void   incrementLevel();

    StatusCondition getStatusCondition() const;
    void            setStatusCondition(StatusCondition newStatusCondition);

    PkmnDef::PkmnDefPtr getDefinition() const;
    void                setDefinition(PkmnDef::PkmnDefPtr const& newDefinition);

    size_t getHP() const;
    float  getPercentageHP() const;
    bool   isKO() const;
    void   setHP(size_t newHp);
    void   decreaseHP(size_t n);
    void   resetHP();

    static size_t accumulatedExpForLevel(size_t level, PkmnDef::GrowthRate growthRate);

    size_t expToNextLevel() const;
    size_t getExp() const;
    float  getPercentageExp() const;
    void   setExp(size_t newExp);
    void   increaseExp(size_t newExp);

    unsigned char getHappiness() const;
    void          setHappiness(unsigned char newHappiness);

    bool   isMale() const;
    bool   isFemale() const;
    Gender getGender() const;
    void   setGender(Gender newGender);

    std::string getDisplayName() const;

    bool isShiny() const;

private:
    std::array<Move::MovePtr, 4>    moves;
    PkmnDef::PkmnDefPtr             definition;
    size_t                          level = 0;
    size_t                          exp   = 0;
    size_t                          hp    = 0;
    std::string                     nickName;
    std::map<PkmnDef::Stat, size_t> IVs;
    std::map<PkmnDef::Stat, size_t> EVs;
    StatusCondition                 statusCondition = StatusCondition::NONE;
    unsigned char                   happiness       = 0;
    Gender                          gender          = Gender::MALE;
    bool                            shiny           = false;
};

#endif // PKMN_H
