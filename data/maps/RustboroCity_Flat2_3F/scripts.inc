RustboroCity_Flat2_3F_MapScripts::
	.byte 0

RustboroCity_Flat2_3F_EventScript_DevonEmployee::
	msgbox RustboroCity_Flat2_3F_Text_PresidentCollectsRareStones, MSGBOX_NPC
	end

RustboroCity_Flat2_3F_EventScript_Woman::
	msgbox RustboroCity_Flat2_3F_Text_PresidentsSonAlsoCollectsRareStones, MSGBOX_NPC
	end

RustboroCity_Flat2_3F_Text_PresidentCollectsRareStones: @ 8216129
	.string "Devon's President likes to collect\n"
	.string "rare stones.$"

RustboroCity_Flat2_3F_Text_PresidentsSonAlsoCollectsRareStones: @ 8216159
	.string "I think the President's son also\n"
	.string "collects rare stones.$"

