RustboroCity_House2_MapScripts::
	.byte 0

RustboroCity_House2_EventScript_PokefanF::
	msgbox RustboroCity_House2_Text_TrainerSchoolExcellent, MSGBOX_NPC
	end

RustboroCity_House2_EventScript_LittleGirl::
	msgbox RustboroCity_House2_Text_RoxanneKnowsALot, MSGBOX_NPC
	end

RustboroCity_House2_Text_TrainerSchoolExcellent: @ 8215EC6
	.string "The Trainer's School is excellent.\p"
	.string "If you study there, you could even\n"
	.string "become a Gym Leader.$"

RustboroCity_House2_Text_RoxanneKnowsALot: @ 8215F21
	.string "Roxanne, the Gym Leader, really knows\n"
	.string "a lot about Digimon.\p"
	.string "She's really strong, too!$"

