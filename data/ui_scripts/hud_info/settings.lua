local pcoptions = require("LUI.PCOptions")

game:addlocalizedstring("LUA_MENU_FPS", "FPS Counter")
game:addlocalizedstring("LUA_MENU_FPS_DESC", "Show FPS Counter")

game:addlocalizedstring("LUA_MENU_LATENCY", "Server Latency")
game:addlocalizedstring("LUA_MENU_LATENCY_DESC", "Show server latency")

pcoptions.VideoOptionsFeeder = function()
	local items = {
		pcoptions.OptionFactory(
			"ui_r_displayMode",
			"@LUA_MENU_DISPLAY_MODE",
			nil,
			{
				{
					text = "@LUA_MENU_MODE_FULLSCREEN",
					value = "fullscreen"
				},
				{
					text = "@LUA_MENU_MODE_WINDOWED_NO_BORDER",
					value = "windowed_no_border"
				},
				{
					text = "@LUA_MENU_MODE_WINDOWED",
					value = "windowed"
				}
			},
			nil,
			true
		),
		pcoptions.SliderOptionFactory(
			"profileMenuOption_blacklevel",
			"@MENU_BRIGHTNESS",
			"@MENU_BRIGHTNESS_DESC1",
			SliderBounds.PCBrightness.Min,
			SliderBounds.PCBrightness.Max,
			SliderBounds.PCBrightness.Step,
			function(element)
				element:processEvent({
					name = "brightness_over",
					immediate = true
				})
			end,
			function(element)
				element:processEvent({
					name = "brightness_up",
					immediate = true
				})
			end,
			true,
			nil,
			"brightness_updated"
		),
		pcoptions.OptionFactoryProfileData(
			"renderColorBlind",
			"profile_toggleRenderColorBlind",
			"@LUA_MENU_COLORBLIND_FILTER",
			"@LUA_MENU_COLOR_BLIND_DESC",
			{
				{
					text = "@LUA_MENU_ENABLED",
					value = true
				},
				{
					text = "@LUA_MENU_DISABLED",
					value = false
				}
			},
			nil,
			false
		)
	}

	if Engine.IsMultiplayer() and not Engine.IsZombiesMode() then
		table.insert(items, pcoptions.OptionFactory(
			"cg_paintballFx",
			"@LUA_MENU_PAINTBALL",
			"@LUA_MENU_PAINTBALL_DESC",
			{
				{
					text = "@LUA_MENU_ENABLED",
					value = true
				},
				{
					text = "@LUA_MENU_DISABLED",
					value = false
				}
			},
			nil,
			false,
			false
		))
	end

	table.insert(items, pcoptions.OptionFactory(
		"cg_infobar_ping",
		"@LUA_MENU_LATENCY",
		"@LUA_MENU_LATENCY_DESC",
		{
			{
				text = "@LUA_MENU_ENABLED",
				value = true
			},
			{
				text = "@LUA_MENU_DISABLED",
				value = false
			}
		},
		function()
			Engine.GetLuiRoot():processEvent({
				name = "update_hud_infobar_settings"
			})
		end,
		false,
		false
	))

	table.insert(items, pcoptions.OptionFactory(
		"cg_infobar_fps",
		"@LUA_MENU_FPS",
		"@LUA_MENU_FPS_DESC",
		{
			{
				text = "@LUA_MENU_ENABLED",
				value = true
			},
			{
				text = "@LUA_MENU_DISABLED",
				value = false
			}
		},
		function()
			Engine.GetLuiRoot():processEvent({
				name = "update_hud_infobar_settings"
			})
		end,
		false,
		false
	))

	table.insert(items, {
		type = "UIGenericButton",
		id = "option_advanced_video",
		properties = {
			style = GenericButtonSettings.Styles.GlassButton,
			button_text = Engine.Localize("@LUA_MENU_ADVANCED_VIDEO"),
			desc_text = "",
			button_action_func = pcoptions.ButtonMenuAction,
			text_align_without_content = LUI.Alignment.Left,
			menu = "advanced_video"
		}
	})

	return items
end
