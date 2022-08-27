if (game:issingleplayer() or Engine.InFrontend()) then
	return
end

function GetPartyMaxPlayers()
	return Engine.GetDvarInt("sv_maxclients")
end

local scoreboard = LUI.mp_hud.Scoreboard

scoreboard.maxPlayersOnTeam = GetTeamLimitForMaxPlayers(GetPartyMaxPlayers())

scoreboard.scoreColumns.ping = {
	width = Engine.IsZombiesMode() and 90 or 60,
	title = "LUA_MENU_PING",
	getter = function (scoreinfo)
		return scoreinfo.ping == 0 and "BOT" or tostring(scoreinfo.ping)
	end
}

local getcolumns = scoreboard.getColumnsForCurrentGameMode
scoreboard.getColumnsForCurrentGameMode = function(a1)
	local columns = getcolumns(a1)

	if (Engine.IsZombiesMode()) then
		table.insert(columns, scoreboard.scoreColumns.ping)
	end

	return columns
end
