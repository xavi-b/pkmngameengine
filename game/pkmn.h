#ifndef PKMN_H
#define PKMN_H

#include "move.h"
#include "pkmndef.h"

#include <array>
#include <vector>

class Pkmn
{
public:
    enum StatusCondition
    {
        NONE,
        BURN,
        FREEZE,
        PARALYSIS,
        POISON,
        BADLY_POISON,
        SLEEP
    };

    static constexpr unsigned char HighFriendship = 160;

    using PkmnPtr = std::shared_ptr<Pkmn>;

    friend void    tag_invoke(js::value_from_tag, js::value& jv, PkmnPtr const& o);
    friend PkmnPtr tag_invoke(js::value_to_tag<PkmnPtr>, js::value const& jv);

    Pkmn(PkmnDef::PkmnDefPtr definition, size_t level);

    std::map<PkmnDef::Stat, size_t> getStats();

    std::map<PkmnDef::Stat, size_t> const& getIVs() const;
    void                                   setIVs(std::map<PkmnDef::Stat, size_t> const& newIVs);

    std::map<PkmnDef::Stat, size_t> const& getEVs() const;
    void                                   setEVs(std::map<PkmnDef::Stat, size_t> const& newEVs);

    std::array<Move::MovePtr, 4> const& getMoves() const;
    std::array<Move::MovePtr, 4>&       getMoves();
    void                                setMoves(std::array<Move::MovePtr, 4> const& newMoves);
    void                                addMovesFromPkmnDef();

    size_t getLevel() const;
    void   incrementLevel();

    StatusCondition getStatusCondition() const;

    PkmnDef::PkmnDefPtr getDefinition() const;
    void                setDefinition(PkmnDef::PkmnDefPtr const& newDefinition);

    size_t getHP() const;
    bool   isKO() const;
    void   setHP(size_t newHp);
    void   decreaseHP(size_t n);
    void   resetHP();

    size_t expToNextLevel();
    size_t getExp() const;
    void   setExp(size_t newExp);
    void   increaseExp(size_t newExp);

    unsigned char getHappiness() const;
    void          setHappiness(unsigned char newHappiness);

    bool getMale() const;
    void setMale(bool newMale);

private:
    std::array<Move::MovePtr, 4>    moves;
    PkmnDef::PkmnDefPtr             definition;
    size_t                          level = 0;
    size_t                          exp   = 0;
    size_t                          hp    = 0;
    std::string                     nickName;
    std::map<PkmnDef::Stat, size_t> IVs;
    std::map<PkmnDef::Stat, size_t> EVs;
    StatusCondition                 statusCondition = NONE;
    unsigned char                   happiness       = 0;
    bool                            male            = true;
};

#endif // PKMN_H
