if (game:issingleplayer()) then
	return
end

function GetGameModeName()
	return Engine.Localize(Engine.TableLookup(GameTypesTable.File, GameTypesTable.Cols.Ref, GameX.GetGameMode(), GameTypesTable.Cols.Name))
end

-- Allow players to change teams in game.
function CanChangeTeam()
	local f9_local0 = GameX.GetGameMode()
	local f9_local1
	if f9_local0 ~= "aliens" and Engine.TableLookup( GameTypesTable.File, GameTypesTable.Cols.Ref, f9_local0, GameTypesTable.Cols.TeamChoice ) == "1" then
		f9_local1 = not MLG.IsMLGSpectator()
	else
		f9_local1 = false
	end
	return f9_local1
end
