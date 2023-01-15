if (game:issingleplayer()) then
	return
end

function GetGameModeName()
    return Engine.Localize(Engine.TableLookup(GameTypesTable.File, GameTypesTable.Cols.Ref, GameX.GetGameMode(), GameTypesTable.Cols.Name))
end
