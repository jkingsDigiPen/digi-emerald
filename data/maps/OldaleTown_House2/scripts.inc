OldaleTown_House2_MapScripts::
	.byte 0

OldaleTown_House2_EventScript_Woman::
	msgbox OldaleTown_House2_Text_PokemonLevelUp, MSGBOX_NPC
	end

OldaleTown_House2_EventScript_Man::
	msgbox OldaleTown_House2_Text_YoullGoFurtherWithStrongPokemon, MSGBOX_NPC
	end

OldaleTown_House2_Text_PokemonLevelUp: @ 81FBF6D
	.string "When Digimon battle, they eventually\n"
	.string "level up and become stronger.$"

OldaleTown_House2_Text_YoullGoFurtherWithStrongPokemon: @ 81FBFB0
	.string "If the Digimon with you become\n"
	.string "stronger, you'll be able to go farther\l"
	.string "away from here.$"

