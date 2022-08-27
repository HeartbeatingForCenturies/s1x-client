local mphud = require("LUI.mp_hud.MPHud")
local barheight = 18
local textheight = 13
local textoffsety = barheight / 2 - textheight / 2

function createinfobar()
	local infobar = LUI.UIElement.new({
		left = 180,
		top = 5,
		height = barheight,
		width = 70,
		leftAnchor = true,
		topAnchor = true
	})

	infobar:registerAnimationState("hud_on", {
		alpha = 1
	})

	infobar:registerAnimationState("hud_off", {
		alpha = 0
	})

	return infobar
end

function populateinfobar(infobar)
	elementoffset = 0

	if (Engine.GetDvarBool("cg_infobar_fps")) then
		infobar:addElement(infoelement({
			label = "FPS: ",
			getvalue = function()
				return game:getfps()
			end,
			width = 70,
			interval = 100
		}))
	end

	if (Engine.GetDvarBool("cg_infobar_ping")) then
		infobar:addElement(infoelement({
			label = "Latency: ",
			getvalue = function()
				return game:getping() .. " ms"
			end,
			width = 115,
			interval = 100
		}))
	end
end

function infoelement(data)
	local container = LUI.UIElement.new({
		bottomAnchor = true,
		leftAnchor = true,
		topAnchor = true,
		width = data.width,
		left = elementoffset
	})

	elementoffset = elementoffset + data.width + 10

	local background = LUI.UIImage.new({
		bottomAnchor = true,
		leftAnchor = true,
		topAnchor = true,
		rightAnchor = true,
		color = {
			r = 0.3,
			g = 0.3,
			b = 0.3,
		},
		material = RegisterMaterial("distort_hud_bkgnd_ui_blur")
	})

	local labelfont = RegisterFont("fonts/bodyFontBold", textheight)

	local label = LUI.UIText.new({
		left = 5,
		top = textoffsety + 1,
		font = labelfont,
		height = textheight,
		leftAnchor = true,
		topAnchor = true,
		color = {
			r = 0.8,
			g = 0.8,
			b = 0.8,
		}
	})

	label:setText(data.label)

	local _, _, left = GetTextDimensions(data.label, labelfont, textheight)
	local value = LUI.UIText.new({
		left = left + 5,
		top = textoffsety,
		font = RegisterFont("fonts/bodyFont", textheight),
		height = textheight + 1,
		leftAnchor = true,
		topAnchor = true,
		color = {
			r = 0.6,
			g = 0.6,
			b = 0.6,
		}
	})
	
	value:addElement(LUI.UITimer.new(data.interval, "update"))
	value:setText(data.getvalue())
	value:addEventHandler("update", function()
		value:setText(data.getvalue())
	end)
	
	container:addElement(background)
	container:addElement(label)
	container:addElement(value)
	
	return container
end

local updatehudvisibility = mphud.updateHudVisibility
mphud.updateHudVisibility = function(a1, a2)
	updatehudvisibility(a1, a2)

	local root = Engine.GetLuiRoot()
	local menus = root:AnyActiveMenusInStack()
	local infobar = root.infobar

	if (not infobar) then
		return
	end

	if (menus) then
		infobar:animateToState("hud_off")
	else
		infobar:animateToState("hud_on")
	end
end

local mphud = LUI.MenuBuilder.m_types_build["mp_hud"]
LUI.MenuBuilder.m_types_build["mp_hud"] = function()
	local hud = mphud()

	if (Engine.InFrontend()) then
		return hud
	end

	local infobar = createinfobar()
	local root = Engine.GetLuiRoot()
	root.infobar = infobar
	populateinfobar(infobar)

	root:registerEventHandler("update_hud_infobar_settings", function()
		infobar:removeAllChildren()
		populateinfobar(infobar)
	end)
	
	hud.static:addElement(infobar)

	return hud
end
