function luiglobals.GetPartyMaxPlayers()
    return Engine.GetDvarInt("sv_maxclients")
end

LUI.mp_hud.Scoreboard.maxPlayersOnTeam = luiglobals.GetTeamLimitForMaxPlayers(luiglobals.GetPartyMaxPlayers())