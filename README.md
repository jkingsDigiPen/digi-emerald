# Digi-Emerald

## What is Digi-Emerald?

This is a Pokemon to Digimon conversion project, using a modified Pokemon Emerald as a base. This began as a fork of laserXdolphin's excellent PokeEmerald Expansions Base, a modification of the Pokemon Emerald decomp that includes many feature branches and changes that drastically improve the quality of life of the base Pokemon Emerald game. My aim is to build off of these features to create a Pokemon game featuring Digimon instead of Pokemon. Aside from the aforementioned QoL changes, the conversion of Pokemon to Digimon will primarily be aesthetic. If I have time, I will make additional changes (listed below), but without drastically altering the difficulty or pacing of the game.

## Planned Changes

- Replace names, battle sprites, icons, and dex entries for approximately 212 Pokemon (roughly the number obtainable in the original game) with Digimon, beginning with starters.
- Change all encounters and trainer battles to utilize altered mons while preserving the flow and balance of the game. This will be done roughly linearly, with areas at the beginning of the game receiving the most attention initially.
- Change base stat totals and abilities so that they make sense for the Digimon.
- Alter movesets where necessary, without adding new moves or changing existing moves beyond what's included in the ROM base.
- Alter all dialog and item text to refer to Digimon rather than Pokemon.
- New title screen to make sure people know what they're getting into.

## Possible Additions

These are not likely to happen unless I have an abundance of time or assistance with the project, but would be nice to have.

- Make every Digimon useful/competitive. This will of course change the feel and balance of the game, but is something that has been a pet peeve of mine in many monster catching games. My goal would be to do this *without* drastically altering the difficulty of the game.
- Change moves or add additional moves to bring them more in line with traditional Digimon moves.
- Change overworld sprites for Pokemon to Digimon.

# Changes from laserXdolphin's Base ROM

The following text relates to changes to the base ROM (not my changes), and come directly from laserXdolphin's readme. See below for changes included when this project was initially forked from their base. If I merge/integrate additional changes at a later point in time, they will also be listed here.

## What feature branches are included?

- **[RHH's Battle Engine Upgrade](../tree/battle_engine):** Upgrades the battle engine in pokeemerald to newer Generation games' standards. It also adds newer moves and abilities.
- **[RHH's Pokémon Expansion](../tree/pokemon_expansion):** Adds Pokémon from newer Generations and makes them available in the National Dex. It also updates base stats and other Pokémon info.
- **[RHH's Item Expansion](../tree/item_expansion):** Adds items from newer Generations and also updates item effects for field use. (Removed TM51 - TM100)
- **[TheXaman's PokedexPlus with a HGSS style](https://www.pokecommunity.com/showthread.php?t=441996):** 
- **[ghoulslash's DexNav & Detector Mode](https://www.pokecommunity.com/showthread.php?t=440571):**
- **[Some of ghoulslash's free_saveblock](https://www.pokecommunity.com/showthread.php?p=10168472#post10168472):** Namely FREE_UNION_ROOM_CHAT, FREE_LINK_BATTLE_RECORDS, FREE_RECORD_MIXING_HALL_RECORDS and APPRENTICE_COUNT set to 1 instead of 4 as well as MAIL_COUNT set to (PARTY_SIZE) instead of (10 + PARTY_SIZE)
- **[Prof. Leon Dias' Fully Decapitalized Pokeemerald](https://www.pokecommunity.com/showthread.php?t=451448)**
- **[Fixes and content from the pret tutorials](https://github.com/pret/pokeemerald/wiki/Tutorials):** Indoor running, BW Repel System, Trainer Classed Pokeballs, Map dependent Trainer Battle Music
- **[Content from Pokecommunity Simple Modifications Directory Content](https://www.pokecommunity.com/showthread.php?t=416647):** Setting EVs and Abilities for trainer battles, Nature Coloring and IV/EV Stat screens 

## Pokecommunity Simple Modifications Directory Content
- [Soft Level Cap - PokemonCrazy](https://www.pokecommunity.com/showthread.php?t=435445)
- Set a Trainer's Pokémon's [EVs - lightbox87](https://www.pokecommunity.com/showpost.php?p=10135792), [abilities - Diego Mertens](https://www.pokecommunity.com/showpost.php?p=10127338) and nature
- [Nature Color - DizzyEgg](https://www.pokecommunity.com/showpost.php?p=10024409)
- [Showing IVs/EVs in Summary Screen - PokemonCrazy](https://www.pokecommunity.com/showpost.php?p=10161688) using L, R and Start
- [Item field effect functions to increase/decrease Individual Values - Lunos](https://www.pokecommunity.com/showpost.php?p=10469674) Vitamins affect IVs
- [Restoring the Shred Split Battle Transition - Lunos](https://www.pokecommunity.com/showpost.php?p=10449556)
- [Poisonpoint no Overworld Poison Damage - Lunos](https://www.pokecommunity.com/showpost.php?p=10349195) 
- "Move Pokémon" as first PSS option
- [Improving the Pace of Battles - Mkol103](https://www.pokecommunity.com/showpost.php?p=10266925)
- [Wrapping Summary Screen - Zeturic](https://www.pokecommunity.com/showpost.php?p=10060875)
- [Move Item - Zeturic](https://www.pokecommunity.com/showpost.php?p=10120157)
- [DPPt Bike (with L) - Lunos](https://www.pokecommunity.com/showpost.php?p=10217718)
- [Faster Surfing - ghoulslash](https://www.pokecommunity.com/showpost.php?p=10137446)
- [Berry Trees No Longer Disappear - Buffel Saft](https://www.pokecommunity.com/showpost.php?p=10142996&postcount=63)
- [Remove the need to water berries on rainy Routes - Buffel Saft](https://www.pokecommunity.com/showpost.php?p=10349397&postcount=258)
- [Item Descriptions On First Obtain - ghoulslash](https://www.pokecommunity.com/showpost.php?p=10126502)
- [Custom Battle Music Via Scripting - Anon822](https://www.pokecommunity.com/showpost.php?p=10341282)
- [Power item and Destiny Knot breeding - DarkDown](https://www.pokecommunity.com/showpost.php?p=10488226)

## Pret Tutorial Content

### Fixes to vanilla code:
- Uncommented bugfixes in config.h
- [Surf Dismount Ground Effects - ghoulslash](https://www.pokecommunity.com/showpost.php?p=10184839)
- [Fix Snow Weather - ghoulslash](https://www.pokecommunity.com/showpost.php?p=10182047)
- Seeding the RNG properly on startup
- Keep the Camera from Making Waves
- Not showing dex entries until getting the Pokédex
- Improving the WaitForVBlank function
- [Better Reflection System - ghoulslash](https://www.pokecommunity.com/showpost.php?p=10473906)

### Removing content:
- Update obedience levels to current standards
- Disabling Union Room check when entering Pokémon Centers

### Features from other generations
- [Remove the extra save confirmation - Hiroshi Sotomura](https://www.pokecommunity.com/showpost.php?p=10211835)
- Allow running indoors
- [Trainer Class-Based Poké Balls - Buffel Saft](https://www.pokecommunity.com/showpost.php?p=10038105)
- Prompt for reusing Repels
- [LGPE-Style Bonus Premier Balls - Buffel Saft](https://www.pokecommunity.com/showpost.php?p=10178662)
- Make the keyboard auto-switch to lowercase after the first character
- [Pokemart Items by Badge Count - ghoulslash](https://www.pokecommunity.com/showpost.php?p=10172995&postcount=96)

### pokeemerald Miscellaneous
- [Disable Bag Use in Battle - ghoulslash](https://www.pokecommunity.com/showpost.php?p=10184292&postcount=122)
- Speedy Pokecenter Healing
- Battle music changes depending on the map the player is in
- Extra save space with two lines of code
- Enable the Reset RTC Feature
