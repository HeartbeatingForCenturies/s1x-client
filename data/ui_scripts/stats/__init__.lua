if (game:issingleplayer() or not Engine.InFrontend()) then
	return
end

game:addlocalizedstring("LUA_MENU_STATS", "Stats")
game:addlocalizedstring("LUA_MENU_STATS_DESC", "Edit player stats settings.")

game:addlocalizedstring("LUA_MENU_UNLOCKALL_ITEMS", "Unlock All Items")
game:addlocalizedstring("LUA_MENU_UNLOCKALL_ITEMS_DESC",
	"Whether items should be locked based on the player's stats or always unlocked.")

game:addlocalizedstring("LUA_MENU_UNLOCKALL_CLASSES", "Unlock All Classes")
game:addlocalizedstring("LUA_MENU_UNLOCKALL_CLASSES_DESC",
	"Whether classes should be locked based on the player's stats or always unlocked.")

game:addlocalizedstring("LUA_MENU_PRESTIGE", "Prestige")
game:addlocalizedstring("LUA_MENU_PRESTIGE_DESC", "Edit prestige level.")
game:addlocalizedstring("LUA_MENU_RANK", "Rank")
game:addlocalizedstring("LUA_MENU_RANK_DESC", "Edit rank.")

local armorybutton = LUI.MPLobbyBase.AddArmoryButton
LUI.MPLobbyBase.AddArmoryButton = function(menu)
	armorybutton(menu)
	menu:AddButton("@LUA_MENU_STATS", function(a1, a2)
		LUI.FlowManager.RequestAddMenu(a1, "menu_stats", true, nil)
	end)
end

-- button stuff for configuring
function IsEnabled(dvar)
	local enabled = Engine.GetDvarBool(dvar)
	if enabled then
		return Engine.Localize("@LUA_MENU_ENABLED")
	end

	return Engine.Localize("@LUA_MENU_DISABLED")
end

function ToggleEnable(dvar)
	local enabled = Engine.GetDvarBool(dvar)
	if enabled then
		Engine.SetDvarBool(dvar, false)
	else
		Engine.SetDvarBool(dvar, true)
	end
end

function GoDirection(dvar, direction, callback)
	local value = Engine.GetDvarString(dvar)
	value = tonumber(value)

	-- get rank data
	local max = nil
	if (dvar == "ui_rank_level_") then
		max = Rank.GetMaxRank(CoD.PlayMode.Core)
	elseif (dvar == "ui_prestige_level") then
		max = Lobby.GetMaxPrestigeLevel()
	end

	local new_value = nil
	if (direction == "down") then
		new_value = value - 1
	elseif (direction == "up") then
		new_value = value + 1
	end

	-- checking to make sure its < 0 or > max
	if (new_value < 0) then
		new_value = max
	elseif (new_value > max) then
		new_value = 0
	end

	callback(new_value)

	Engine.SetDvarFromString(dvar, new_value .. "")
end

LUI.MenuBuilder.registerType("menu_stats", function(a1, a2)
	local menu = LUI.MenuTemplate.new(a1, {
		menu_title = Engine.ToUpperCase(Engine.Localize("@LUA_MENU_STATS")),
		menu_width = GenericMenuDims.menu_right_wide - GenericMenuDims.menu_left,
		menu_height = 548
	})

	menu:setClass(LUI.Options)
	menu.controller = a2.exclusiveController

	local itemsbutton = menu:AddButtonVariant(GenericButtonSettings.Variants.Select,
		"@LUA_MENU_UNLOCKALL_ITEMS", "@LUA_MENU_UNLOCKALL_ITEMS_DESC", function()
			return IsEnabled("cg_unlockall_items")
		end, function()
			ToggleEnable("cg_unlockall_items")
		end, function()
			ToggleEnable("cg_unlockall_items")
		end)

	local classesbutton = menu:AddButtonVariant(GenericButtonSettings.Variants.Select,
		"@LUA_MENU_UNLOCKALL_CLASSES", "@LUA_MENU_UNLOCKALL_CLASSES_DESC", function()
			return IsEnabled("cg_unlockall_classes")
		end, function()
			ToggleEnable("cg_unlockall_classes")
		end, function()
			ToggleEnable("cg_unlockall_classes")
		end)

	local prestige = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "prestige") or 0
	local experience = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "experience") or 0
	local rank = AAR.GetRankForXP(experience, prestige)

	local prestigevalue = prestige
	local rankvalue = rank

	-- save changes made
	local save_changes = function()
		Engine.SetPlayerDataEx(0, CoD.StatsGroup.Ranked, "prestige", tonumber(prestigevalue))

		local rank = tonumber(rankvalue)
		local prestige = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "prestige") or 0
		local experience = rank == 0 and 0 or Rank.GetRankMaxXP(tonumber(rankvalue) - 1, prestige)

		Engine.SetPlayerDataEx(0, CoD.StatsGroup.Ranked, "experience", experience)
	end

	-- back callback
	local back = function()
		save_changes()
		LUI.common_menus.Options.HideOptionsBackground()
		LUI.FlowManager.RequestLeaveMenu(menu)
	end

	-- create buttons and create callbacks
	CreateEditButton(menu, "ui_prestige_level", "@LUA_MENU_PRESTIGE", "@LUA_MENU_PRESTIGE_DESC", function(value)
		prestigevalue = value
	end)
	CreateEditButton(menu, "ui_rank_level_", "@LUA_MENU_RANK", "@LUA_MENU_RANK_DESC", function(value)
		rankvalue = value
	end)

	menu:AddBottomDescription(menu:InitScrolling())
	menu:AddBackButton(back)

	LUI.common_menus.Options.ShowOptionsBackground()

	return menu
end)

function CreateEditButton(menu, dvar, name, desc, callback)
	local prestige = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "prestige") or 0
	local experience = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "experience") or 0

	local dvarValue = nil
	local max = nil
	if (dvar == "ui_rank_level_") then
		dvarValue = AAR.GetRankForXP(experience, prestige)
		max = Rank.GetMaxRank(CoD.PlayMode.Core)
	elseif (dvar == "ui_prestige_level") then
		dvarValue = prestige
		max = Lobby.GetMaxPrestigeLevel()
	end

	Engine.SetDvarFromString(dvar, dvarValue .. "")

	menu:AddButtonVariant(GenericButtonSettings.Variants.Select, name, desc, function()
		local dvarString = Engine.GetDvarString(dvar)

		if (dvar == "ui_rank_level_") then
			dvarString = tonumber(dvarString) + 1
		end

		return tostring(dvarString)
	end, function()
		GoDirection(dvar, "down", callback)
	end, function()
		GoDirection(dvar, "up", callback)
	end)
end

local isclasslocked = Cac.IsCustomClassLocked
Cac.IsCustomClassLocked = function(...)
	if (Engine.GetDvarBool("cg_unlockall_classes")) then
		return false
	end

	return isclasslocked(table.unpack({...}))
end

local isdlcclasslocked = Cac.IsCustomClassDlcLocked
Cac.IsCustomClassDlcLocked = function(...)
	if (Engine.GetDvarBool("cg_unlockall_classes")) then
		return false
	end

	return isdlcclasslocked(table.unpack({...}))
end
