#!/usr/bin/env python3
"""Rewrite game/resources/data.json down to the FireRed roster.

Reads a local pokefirered checkout (not vendored) and overwrites battle fields
for the species, moves, abilities, and items that exist in both trees.
"""

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DATA_PATH = ROOT / "game" / "resources" / "data.json"

SPECIES_RENAMES = {
    "NIDORANF": "NIDORANfE",
    "NIDORANM": "NIDORANmA",
}
MOVE_RENAMES = {
    "FAINTATTACK": "FEINTATTACK",
    "HIJUMPKICK": "HIGHJUMPKICK",
    "SMELLINGSALT": "SMELLINGSALTS",
    "VICEGRIP": "VISEGRIP",
}
ITEM_RENAMES = {
    "XDEFEND": "XDEFENSE",
    "XSPECIAL": "XSPATK",
}

# PkmnDef::Stat
STAT_HP, STAT_ATK, STAT_DEF, STAT_SPA, STAT_SPD, STAT_SPE = range(6)
# PkmnDef::EvolutionType
EVO_ATTACK_GREATER = 0
EVO_DEFENSE_GREATER = 1
EVO_ATK_DEF_EQUAL = 2
EVO_HAPPINESS = 4
EVO_HAPPINESS_DAY = 5
EVO_HAPPINESS_NIGHT = 6
EVO_ITEM = 9
EVO_LEVEL = 10
EVO_EMPTY_SLOT = 14
EVO_TRADE = 15
EVO_TRADE_ITEM = 16
# MoveDef::Category / Target
CAT_PHYSICAL, CAT_SPECIAL, CAT_STATUS = 0, 1, 2
TARGET = {
    "MOVE_TARGET_SELECTED": 9,          # NearOther
    "MOVE_TARGET_DEPENDS": 9,           # NearOther
    "MOVE_TARGET_BOTH": 3,              # AllNearFoes
    "MOVE_TARGET_RANDOM": 11,           # RandomNearFoe
    "MOVE_TARGET_USER": 12,             # User
    "MOVE_TARGET_FOES_AND_ALLY": 4,     # AllNearOthers
    "MOVE_TARGET_OPPONENTS_FIELD": 6,   # FoeSide
}
PHYSICAL_TYPES = {
    "NORMAL", "FIGHTING", "FLYING", "POISON", "GROUND", "ROCK", "BUG", "GHOST", "STEEL",
}
SPECIAL_TYPES = {
    "FIRE", "WATER", "GRASS", "ELECTRIC", "PSYCHIC", "ICE", "DRAGON", "DARK",
}
TYPE_NAMES = {
    "NORMAL": "NORMAL",
    "FIGHTING": "FIGHTING",
    "FLYING": "FLYING",
    "POISON": "POISON",
    "GROUND": "GROUND",
    "ROCK": "ROCK",
    "BUG": "BUG",
    "GHOST": "GHOST",
    "STEEL": "STEEL",
    "MYSTERY": "QMARKS",
    "FIRE": "FIRE",
    "WATER": "WATER",
    "GRASS": "GRASS",
    "ELECTRIC": "ELECTRIC",
    "PSYCHIC": "PSYCHIC",
    "ICE": "ICE",
    "DRAGON": "DRAGON",
    "DARK": "DARK",
}
FR_EVO = {
    "FRIENDSHIP": (EVO_HAPPINESS, None),
    "FRIENDSHIP_DAY": (EVO_HAPPINESS_DAY, None),
    "FRIENDSHIP_NIGHT": (EVO_HAPPINESS_NIGHT, None),
    "LEVEL": (EVO_LEVEL, "param"),
    "TRADE": (EVO_TRADE, None),
    "TRADE_ITEM": (EVO_TRADE_ITEM, "item"),
    "ITEM": (EVO_ITEM, "item"),
    "LEVEL_ATK_GT_DEF": (EVO_ATTACK_GREATER, "param"),
    "LEVEL_ATK_EQ_DEF": (EVO_ATK_DEF_EQUAL, "param"),
    "LEVEL_ATK_LT_DEF": (EVO_DEFENSE_GREATER, "param"),
    "LEVEL_SILCOON": (EVO_LEVEL, "param"),
    "LEVEL_CASCOON": (EVO_LEVEL, "param"),
    "LEVEL_NINJASK": (EVO_LEVEL, "param"),
    "LEVEL_SHEDINJA": (EVO_EMPTY_SLOT, "param"),
    "BEAUTY": (EVO_HAPPINESS, None),
}


def norm(token):
    return token.replace("_", "").upper()


def game_id(token, renames):
    key = norm(token)
    return renames.get(key, key)


def defines(path, prefix):
    found = {}
    for line in path.read_text(errors="replace").splitlines():
        match = re.match(rf"#define {prefix}([A-Z0-9_]+)\s+(-?\d+|0x[0-9A-Fa-f]+)", line)
        if match and match.group(1) not in found:
            found[match.group(1)] = int(match.group(2), 0)
    return found


def blocks(text, kind):
    """Map SYMBOL -> brace body for `[KIND_SYMBOL] = { ... }`."""
    out = {}
    for match in re.finditer(rf"\[{kind}_([A-Z0-9_]+)\]\s*=\s*\{{", text):
        name = match.group(1)
        start = match.end()
        depth = 1
        i = start
        while i < len(text) and depth:
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
            i += 1
        out[name] = text[start : i - 1]
    return out


def field(body, name):
    match = re.search(rf"\.{name}\s*=\s*(\{{[^}}]*\}}|[^,\n]+)", body)
    return match.group(1).strip() if match else None


def national_order(path):
    names = []
    for line in path.read_text(errors="replace").splitlines():
        match = re.match(r"\s*NATIONAL_DEX_([A-Z0-9_]+)\s*,", line)
        if match:
            names.append(match.group(1))
    return names


def skip_species(name):
    return (
        name in {"NONE", "EGG"}
        or name.startswith("OLD_UNOWN")
        or (name.startswith("UNOWN_") and name != "UNOWN")
    )


def load_learnsets(fr):
    text = (fr / "src/data/pokemon/level_up_learnsets.h").read_text(errors="replace")
    tables = {}
    for match in re.finditer(
        r"static const u16 (s\w+LevelUpLearnset)\[\] = \{(.*?)\};", text, re.S
    ):
        moves = []
        for level, move in re.findall(r"LEVEL_UP_MOVE\(\s*(\d+)\s*,\s*MOVE_([A-Z0-9_]+)\s*\)", match.group(2)):
            moves.append((int(level), move))
        tables[match.group(1)] = moves
    pointers = (fr / "src/data/pokemon/level_up_learnset_pointers.h").read_text(errors="replace")
    by_species = {}
    for species, table in re.findall(r"\[SPECIES_([A-Z0-9_]+)\]\s*=\s*(s\w+LevelUpLearnset)", pointers):
        by_species[species] = tables.get(table, [])
    return by_species


def load_tmhm(fr):
    """Move ids each species can learn from a TM or HM, separate from level-up."""
    text = (fr / "src/data/pokemon/tmhm_learnsets.h").read_text(errors="replace")
    by_species = {}
    for match in re.finditer(
        r"\[SPECIES_([A-Z0-9_]+)\]\s*=\s*TMHM_LEARNSET\((.*?)\)\s*,", text, re.S
    ):
        moves = []
        for token in re.findall(r"TMHM\((?:TM|HM)\d+_([A-Z0-9_]+)\)", match.group(2)):
            move_id = game_id(token, MOVE_RENAMES)
            if move_id not in moves:
                moves.append(move_id)
        by_species[match.group(1)] = moves
    return by_species


def load_evolutions(fr, species_ids):
    text = (fr / "src/data/pokemon/evolution.h").read_text(errors="replace")
    by_species = {name: [] for name in species_ids}
    for species, body in blocks(text, "SPECIES").items():
        if species not in by_species:
            continue
        for method, param, target in re.findall(
            r"\{EVO_([A-Z0-9_]+)\s*,\s*([^,]+)\s*,\s*SPECIES_([A-Z0-9_]+)\s*\}", body
        ):
            by_species[species].append((method, param.strip(), target))
    return by_species


def category_for(move_type, power):
    if power <= 0:
        return CAT_STATUS
    if move_type in PHYSICAL_TYPES:
        return CAT_PHYSICAL
    if move_type in SPECIAL_TYPES:
        return CAT_SPECIAL
    return CAT_STATUS


def main():
    if len(sys.argv) != 2:
        sys.exit("usage: trim_to_firered.py /path/to/pokefirered")
    fr = Path(sys.argv[1])
    species_ids = defines(fr / "include/constants/species.h", "SPECIES_")
    move_ids = defines(fr / "include/constants/moves.h", "MOVE_")
    ability_ids = defines(fr / "include/constants/abilities.h", "ABILITY_")
    item_ids = defines(fr / "include/constants/items.h", "ITEM_")

    real_species = [
        name
        for name in national_order(fr / "include/constants/pokedex.h")
        if name in species_ids and not skip_species(name)
    ]
    info = blocks((fr / "src/data/pokemon/species_info.h").read_text(errors="replace"), "SPECIES")
    pokedex = blocks((fr / "src/data/pokemon/pokedex_entries.h").read_text(errors="replace"), "NATIONAL_DEX")
    moves = blocks((fr / "src/data/battle_moves.h").read_text(errors="replace"), "MOVE")
    learnsets = load_learnsets(fr)
    tmhm = load_tmhm(fr)
    evolutions = load_evolutions(fr, set(real_species))

    data = json.loads(DATA_PATH.read_text())
    pkmn_by_id = {p["id"]: p for p in data["pkmns"]}
    move_by_id = {m["id"]: m for m in data["moves"]}
    ability_by_id = {a["id"]: a for a in data["abilities"]}
    item_by_id = {i["id"]: i for i in data["items"]}

    kept_species = []
    missing_species = []
    for name in real_species:
        gid = game_id(name, SPECIES_RENAMES)
        if gid not in pkmn_by_id:
            missing_species.append(gid)
            continue
        body = info.get(name)
        if not body:
            missing_species.append(gid)
            continue
        entry = pkmn_by_id[gid]
        type_tokens = re.findall(r"TYPE_([A-Z0-9_]+)", field(body, "types") or "")
        types = []
        for token in type_tokens:
            mapped = TYPE_NAMES.get(token, token)
            if mapped not in types:
                types.append(mapped)
        entry["types"] = types
        entry["stats"] = [
            {"id": STAT_HP, "value": int(field(body, "baseHP") or 0)},
            {"id": STAT_ATK, "value": int(field(body, "baseAttack") or 0)},
            {"id": STAT_DEF, "value": int(field(body, "baseDefense") or 0)},
            {"id": STAT_SPA, "value": int(field(body, "baseSpAttack") or 0)},
            {"id": STAT_SPD, "value": int(field(body, "baseSpDefense") or 0)},
            {"id": STAT_SPE, "value": int(field(body, "baseSpeed") or 0)},
        ]
        abilities = []
        for token in re.findall(r"ABILITY_([A-Z0-9_]+)", field(body, "abilities") or ""):
            if token == "NONE":
                continue
            aid = game_id(token, {})
            if aid not in abilities:
                abilities.append(aid)
        entry["abilities"] = abilities
        entry["hiddenAbilities"] = []
        entry["catchRate"] = int(field(body, "catchRate") or 0)
        entry["baseExp"] = int(field(body, "expYield") or 0)
        weight = field(pokedex.get(name, ""), "weight")
        if weight is not None:
            entry["weight"] = int(weight) / 10.0
        learned = []
        for level, move_name in learnsets.get(name, []):
            mid = game_id(move_name, MOVE_RENAMES)
            learned.append({"level": level, "move": mid})
        entry["movesToLearn"] = learned
        entry["machines"] = list(tmhm.get(name, []))
        evos = []
        for method, param, target in evolutions.get(name, []):
            spec = FR_EVO.get(method)
            if spec is None:
                continue
            evo_type, kind = spec
            target_id = game_id(target, SPECIES_RENAMES)
            if kind == "param":
                payload = re.sub(r"\s", "", param)
            elif kind == "item":
                payload = game_id(param.removeprefix("ITEM_"), ITEM_RENAMES)
            else:
                payload = ""
            evos.append({"type": evo_type, "pkmnId": target_id, "data": payload})
        entry["evolutions"] = evos
        kept_species.append(entry)

    kept_moves = []
    missing_moves = []
    for name, _index in sorted(move_ids.items(), key=lambda kv: kv[1]):
        if name in {"NONE", "STRUGGLE", "UNAVAILABLE"}:
            continue
        gid = game_id(name, MOVE_RENAMES)
        entry = move_by_id.get(gid)
        body = moves.get(name)
        if entry is None or body is None:
            missing_moves.append(gid)
            continue
        move_type = TYPE_NAMES[(field(body, "type") or "TYPE_NORMAL").removeprefix("TYPE_")]
        power = int(field(body, "power") or 0)
        entry["type"] = move_type
        entry["power"] = power
        entry["accuracy"] = int(field(body, "accuracy") or 0)
        entry["totalPP"] = int(field(body, "pp") or 0)
        entry["priority"] = int(field(body, "priority") or 0)
        # Pret stores the Gen V value. Cartridge FireRed Roar and Whirlwind are priority 0.
        if name in {"ROAR", "WHIRLWIND"}:
            entry["priority"] = 0
        entry["effectChance"] = int(field(body, "secondaryEffectChance") or 0)
        entry["category"] = category_for(move_type, power)
        target = field(body, "target")
        if target in TARGET:
            entry["target"] = TARGET[target]
        kept_moves.append(entry)

    kept_move_ids = {m["id"] for m in kept_moves}
    for entry in kept_species:
        entry["movesToLearn"] = [row for row in entry["movesToLearn"] if row["move"] in kept_move_ids]
        entry["machines"] = [move_id for move_id in entry.get("machines", []) if move_id in kept_move_ids]
        entry["evolutions"] = [row for row in entry["evolutions"] if row["pkmnId"] in {s["id"] for s in kept_species}]

    kept_abilities = []
    for name, _index in sorted(ability_ids.items(), key=lambda kv: kv[1]):
        if name in {"NONE", "CACOPHONY"}:
            continue
        gid = game_id(name, {})
        if gid in ability_by_id:
            kept_abilities.append(ability_by_id[gid])

    kept_ability_ids = {a["id"] for a in kept_abilities}
    for entry in kept_species:
        entry["abilities"] = [aid for aid in entry["abilities"] if aid in kept_ability_ids]

    kept_items = []
    for name, _index in sorted(item_ids.items(), key=lambda kv: kv[1]):
        if name == "NONE" or name.startswith("TM") and name.endswith("_MOVE"):
            continue
        if name.startswith("HM") and name.endswith("_MOVE"):
            continue
        gid = game_id(name, ITEM_RENAMES)
        if gid in item_by_id:
            kept_items.append(item_by_id[gid])
    # Preserve original item order among the ones that matched.
    kept_item_ids = {i["id"] for i in kept_items}
    kept_items = [i for i in data["items"] if i["id"] in kept_item_ids]

    for type_entry in data["types"]:
        for key in ("weaknesses", "resistances", "immunities"):
            type_entry[key] = [t for t in type_entry[key] if t != "FAIRY"]
        if type_entry["id"] == "STEEL":
            for extra in ("GHOST", "DARK"):
                if extra not in type_entry["resistances"]:
                    type_entry["resistances"].append(extra)
    data["types"] = [t for t in data["types"] if t["id"] != "FAIRY"]

    data["pkmns"] = kept_species
    data["moves"] = kept_moves
    data["abilities"] = kept_abilities
    data["items"] = kept_items

    DATA_PATH.write_text(json.dumps(data, indent=4) + "\n")
    print(
        f"species {len(kept_species)} moves {len(kept_moves)} "
        f"abilities {len(kept_abilities)} items {len(kept_items)}"
    )
    if missing_species:
        print("missing species", missing_species)
    if missing_moves:
        print("missing moves", len(missing_moves), missing_moves[:12])
    codes = sorted({m["functionCode"] for m in kept_moves})
    print("function codes", len(codes))
    print("codes", ",".join(str(c) for c in codes))


if __name__ == "__main__":
    main()
