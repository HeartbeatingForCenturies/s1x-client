if (game:issingleplayer() or not Engine.InFrontend()) then
    return
end

local Options = LUI.common_menus.Options

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
    else
        return Engine.Localize("@LUA_MENU_DISABLED")
    end
end

function ToggleEnable(dvar)
    local enabled = Engine.GetDvarBool(dvar)
    if enabled then
        Engine.SetDvarBool(dvar, false)
    else
        Engine.SetDvarBool(dvar, true)
    end
end

function GoDirection(dvar, type, direction, callback)
    local value = Engine.GetDvarString(dvar)
    value = tonumber(value)

    -- get rank data so we can sanity check their inputs and make sure they aren't going < 0 or > max
    local max = nil
    if (type == "rank") then
        max = luiglobals.Rank.GetMaxRank(CoD.PlayMode.Core)
    elseif (type == "prestige") then
        max = luiglobals.Lobby.GetMaxPrestigeLevel()
    end

    local new_value = nil
    if (direction == "down") then
        new_value = value - 1
        if (new_value < 0) then
            new_value = max
        elseif (new_value > max) then
            new_value = 0
        end
    elseif (direction == "up") then
        new_value = value + 1
        if (new_value < 0) then
            new_value = max
        elseif (new_value > max) then
            new_value = 0
        end
    end
    callback(new_value)

    Engine.SetDvarFromString(dvar, new_value .. "")
end

LUI.MenuBuilder.registerType("menu_stats", function(a1, a2)
    local menu = LUI.MenuTemplate.new(a1, {
        menu_title = Engine.ToUpperCase(Engine.Localize("@LUA_MENU_STATS")),
        menu_width = luiglobals.GenericMenuDims.menu_right_wide - luiglobals.GenericMenuDims.menu_left,
        menu_height = 548
    })

    menu:setClass(LUI.Options)
    menu.controller = a2.exclusiveController

    local itemsbutton = menu:AddButtonVariant(luiglobals.GenericButtonSettings.Variants.Select,
        "@LUA_MENU_UNLOCKALL_ITEMS", "@LUA_MENU_UNLOCKALL_ITEMS_DESC", function()
            return IsEnabled("cg_unlockall_items")
        end, function()
            ToggleEnable("cg_unlockall_items")
        end, function()
            ToggleEnable("cg_unlockall_items")
        end)

    local classesbutton = menu:AddButtonVariant(luiglobals.GenericButtonSettings.Variants.Select,
        "@LUA_MENU_UNLOCKALL_CLASSES", "@LUA_MENU_UNLOCKALL_CLASSES_DESC", function()
            return IsEnabled("cg_unlockall_classes")
        end, function()
            ToggleEnable("cg_unlockall_classes")
        end, function()
            ToggleEnable("cg_unlockall_classes")
        end)

    local prestige = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "prestige") or 0
    local experience = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "experience") or 0
    local rank = luiglobals.AAR.GetRankForXP(experience, prestige)

    local prestigevalue = prestige
    local rankvalue = rank

    -- save changes made
    local save_changes = function()
        Engine.SetPlayerDataEx(0, CoD.StatsGroup.Ranked, "prestige", tonumber(prestigevalue))

        local rank = tonumber(rankvalue)
        local prestige = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "prestige") or 0
        local experience = rank == 0 and 0 or luiglobals.Rank.GetRankMaxXP(tonumber(rankvalue) - 1, prestige)

        Engine.SetPlayerDataEx(0, CoD.StatsGroup.Ranked, "experience", experience)
    end

    -- back callback
    local back = function()
        save_changes()
        Options.HideOptionsBackground()
        LUI.FlowManager.RequestLeaveMenu(menu)
    end

    -- create buttons and create callbacks
    prestigeeditbutton(menu, function(value)
        prestigevalue = value
    end)
    rankeditbutton(menu, function(value)
        rankvalue = value
    end)

    menu:AddBottomDescription(menu:InitScrolling())
    menu:AddBackButton(back)

    Options.ShowOptionsBackground()

    return menu
end)

function prestigeeditbutton(menu, callback)
    local options = {}
    local max = luiglobals.Lobby.GetMaxPrestigeLevel()
    local prestige = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "prestige") or 0

    for i = 0, max do
        game:addlocalizedstring("LUA_MENU_" .. i, i .. "")

        table.insert(options, {
            text = "@" .. i,
            value = i .. ""
        })
    end

    Engine.SetDvarFromString("ui_prestige_level", prestige .. "")

    menu:AddButtonVariant(luiglobals.GenericButtonSettings.Variants.Select, "@LUA_MENU_PRESTIGE",
        "@LUA_MENU_PRESTIGE_DESC", function()
            return Engine.GetDvarString("ui_prestige_level")
        end, function()
            GoDirection("ui_prestige_level", "prestige", "down", callback)
        end, function()
            GoDirection("ui_prestige_level", "prestige", "up", callback)
        end)
end

function rankeditbutton(menu, callback)
    local options = {}
    local prestige = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "prestige") or 0
    local experience = Engine.GetPlayerDataEx(0, CoD.StatsGroup.Ranked, "experience") or 0

    local rank = luiglobals.AAR.GetRankForXP(experience, prestige)
    local max = luiglobals.Rank.GetMaxRank(CoD.PlayMode.Core)

    for i = 0, max do
        game:addlocalizedstring("LUA_MENU_" .. i, i .. "")

        table.insert(options, {
            text = "@" .. (i + 1),
            value = i .. ""
        })
    end

    Engine.SetDvarFromString("ui_rank_level_", rank .. "")

    menu:AddButtonVariant(luiglobals.GenericButtonSettings.Variants.Select, "@LUA_MENU_RANK", "@LUA_MENU_RANK_DESC",
        function()
            -- show 1 more level than it actually is (ex: rank "0" is really rank 1)
            local rank = Engine.GetDvarString("ui_rank_level_")
            rank = tonumber(rank) + 1
            return tostring(rank)
        end, function()
            GoDirection("ui_rank_level_", "rank", "down", callback)
        end, function()
            GoDirection("ui_rank_level_", "rank", "up", callback)
        end)
end

local isclasslocked = luiglobals.Cac.IsCustomClassLocked
luiglobals.Cac.IsCustomClassLocked = function(...)
    if (Engine.GetDvarBool("cg_unlockall_classes")) then
        return false
    end

    return isclasslocked(table.unpack({...}))
end
