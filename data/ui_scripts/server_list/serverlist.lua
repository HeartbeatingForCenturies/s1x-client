if (game:issingleplayer() or not Engine.InFrontend()) then
    return
end

local SystemLinkJoinMenu = LUI.mp_menus.SystemLinkJoinMenu

game:addlocalizedstring("PLATFORM_SYSTEM_LINK_TITLE", "SERVER LIST")
game:addlocalizedstring("MENU_NUMPLAYERS", "Players")
game:addlocalizedstring("MENU_PING", "Ping")

local offsets = {10, 400, 600, 900, 1075}

local columns = {"@MENU_HOST_NAME", "@MENU_MAP", "@MENU_TYPE1", "@MENU_NUMPLAYERS", "@MENU_PING"}

SystemLinkJoinMenu.AddServerButton = function(menu, controller, index)
    local serverInformation = nil
    local button = menu:AddButton("", SystemLinkJoinMenu.OnJoinGame)

    if index == nil then
        button:makeNotFocusable()
        serverInformation = function(i)
            return Engine.Localize(columns[i])
        end
    else
        button:makeFocusable()
        button.index = index
        serverInformation = function(i)
            return Lobby.GetServerData(controller, index, i - 1)
        end
    end

    for size = 1, #offsets do
        SystemLinkJoinMenu.MakeText(button, offsets[size], serverInformation(size))
    end

    return button
end

function menu_systemlink_join(f19_arg0, f19_arg1)
    local menu = LUI.MenuTemplate.new(f19_arg0, {
        menu_title = "@PLATFORM_SYSTEM_LINK_TITLE",
        menu_width = CoD.DesignGridHelper(28)
    })
    Lobby.BuildServerList(Engine.GetFirstActiveController())
    Lobby.RefreshServerList(Engine.GetFirstActiveController())

    SystemLinkJoinMenu.UpdateGameList(menu)
    menu:registerEventHandler("updateGameList", SystemLinkJoinMenu.UpdateGameList)
    menu:addElement(LUI.UITimer.new(250, "updateGameList"))

    menu:AddHelp({
        name = "add_button_helper_text",
        button_ref = "button_alt1",
        helper_text = Engine.Localize("@MENU_SB_TOOLTIP_BTN_REFRESH"),
        side = "right",
        clickable = true,
        priority = -1000
    }, function(f10_arg0, f10_arg1)
        SystemLinkJoinMenu.RefreshServers(f10_arg0, f10_arg1, menu)
    end)

    menu:AddHelp({
        name = "add_button_helper_text",
        button_ref = "button_action",
        helper_text = Engine.Localize("@MENU_JOIN_GAME1"),
        side = "right",
        clickable = false,
        priority = -1000
    })

    menu:AddBackButton()

    return menu
end

LUI.MenuBuilder.m_types_build["menu_systemlink_join"] = menu_systemlink_join
