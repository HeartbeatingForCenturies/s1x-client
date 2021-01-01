#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	/***************************************************************
	 * Functions
	 **************************************************************/

	WEAK symbol<void(errorParm code, const char* message, ...)> Com_Error{ 0x1402F7570, 0x1403CE480 };
	WEAK symbol<void()> Com_Frame_Try_Block_Function{ 0x1402F7E10, 0x1403CEF30 };
	WEAK symbol<void()> Com_Quit_f{ 0x1402F9390, 0x1403D08C0 };

	WEAK symbol<void(const char* cmdName, void(), cmd_function_s* allocedCmd)> Cmd_AddCommandInternal{ 0x1402EDDB0, 0x1403AF2C0 };
	WEAK symbol<void(int localClientNum, int controllerIndex, const char* text)> Cmd_ExecuteSingleCommand{ 0x1402EE350, 0x1403AF900 };
	WEAK symbol<void(const char* text_in)> Cmd_TokenizeString{ 0x1402EEA30, 0x1403B0020 };
	WEAK symbol<void()> Cmd_EndTokenizeString{ 0x1402EE000, 0x1403AF5B0 };

	WEAK symbol<void(const char* message)> Conbuf_AppendText{ 0x14038F220, 0x1404D9040 };

	WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{ 0x1402ED890, 0x1403AED70 };
	WEAK symbol<void(int localClientNum, int controllerIndex, const char* buffer,
		void(int, int, const char*))> Cbuf_ExecuteBufferInternal{ 0x1402ED9A0, 0x1403AEE80 };

	WEAK symbol<void(int localClientNum, const char* message)> CG_GameMessage{ 0x1401A3050, 0x1400EE500 };

	WEAK symbol<dvar_t* (const char* name)> Dvar_FindVar{ 0x140370860, 0x1404BF8B0 };
	WEAK symbol<void(char* buffer, int index)> Dvar_GetCombinedString{ 0x1402FB590, 0x1403D3290 };
	WEAK symbol<void(const char* dvar, const char* buffer)> Dvar_SetCommand{ 0x1403730D0, 0x1404C2520 };
	WEAK symbol<void(dvar_t* dvar, const char* string)> Dvar_SetString{ 0x140373DE0, 0x1404C3610 };
	WEAK symbol<void(const char*, const char*, DvarSetSource)> Dvar_SetFromStringByNameFromSource{ 0x1403737D0, 0x1404C2E40 };
	WEAK symbol<const char* (dvar_t* dvar, dvar_value value)> Dvar_ValueToString{ 0x140374E10, 0x1404C47B0 };

	WEAK symbol<dvar_t* (const char* dvarName, bool value, unsigned int flags, const char* description)>
		Dvar_RegisterBool{ 0x140371850, 0x1404C0BE0 };
	WEAK symbol<dvar_t* (const char* dvarName, const char** valueList, int defaultIndex, unsigned int flags,
		const char* description)> Dvar_RegisterEnum{ 0x140371B30, 0x1404C0EC0 };
	WEAK symbol<dvar_t* (const char* dvarName, float value, float min, float max, unsigned int flags,
		const char* description)> Dvar_RegisterFloat{ 0x140371C20, 0x1404C0FB0 };
	WEAK symbol<dvar_t* (const char* dvarName, int value, int min, int max, unsigned int flags, const char* desc)>
		Dvar_RegisterInt{ 0x140371CF0, 0x1404C1080 };
	WEAK symbol<dvar_t* (const char* dvarName, const char* value, unsigned int flags, const char* description)>
		Dvar_RegisterString{ 0x140372050, 0x1404C1450 };
	WEAK symbol<dvar_t* (const char* dvarName, float x, float y, float z, float w, float min, float max,
		unsigned int flags, const char* description)> Dvar_RegisterVec4{ 0x140372430, 0x1404C1800 };

	WEAK symbol<void()> G_Glass_Update{ 0x14021D540, 0x1402EDEE0 };

	WEAK symbol<unsigned int(int)> Live_SyncOnlineDataFlags{ 0x1404459A0, 0x140562830 };

	WEAK symbol<Material* (const char* material)> Material_RegisterHandle{ 0x1404919D0, 0x1405AFBE0 };

	WEAK symbol<void(float x, float y, float width, float height, float s0, float t0, float s1, float t1,
		float* color, Material* material)> R_AddCmdDrawStretchPic{ 0x1404A2580, 0x1405C0CB0 };
	WEAK symbol<void(const char*, int, Font_s*, float, float, float, float, float, float*, int)> R_AddCmdDrawText{ 0x1404A2BF0, 0x1405C1320 };

	WEAK symbol<void(const char*, int, Font_s*, float, float, float, float, float, const float*, int, int, char)>
		R_AddCmdDrawTextWithCursor{ 0x1404A35E0, 0x1405C1D10 };
	WEAK symbol<Font_s* (const char* font)> R_RegisterFont{ 0x140481F90, 0x14059F3C0 };
	WEAK symbol<int(const char* text, int maxChars, Font_s* font)> R_TextWidth{ 0x140482270, 0x14059F6B0 };

	WEAK symbol<ScreenPlacement* ()> ScrPlace_GetViewPlacement{ 0x14014FA70, 0x14023CB50 };

	WEAK symbol<void(int localClientNum)> SV_FastRestart{ 0, 0x1404374E0 };
	WEAK symbol<void(int, int, const char*)> SV_GameSendServerCommand{ 0, 0x14043E120 };
	WEAK symbol<const char* (int clientNum)> SV_GetGuid{ 0, 0x14043E1E0 };
	WEAK symbol<void(int clientNum, const char* reason)> SV_KickClientNum{ 0, 0x1404377A0 };
	WEAK symbol<bool()> SV_Loaded{ 0x1403F42C0, 0x14043FA50 };
	WEAK symbol<bool(const char* map) >SV_MapExists{ 0, 0x140437800 };
	WEAK symbol<void(int localClientNum, const char* map)> SV_StartMap{ 0, 0x140438320 };
	WEAK symbol<void(int localClientNum, const char* map, bool migrate)> SV_StartMapForParty{ 0, 0x140438490 };

	WEAK symbol<bool()> Sys_IsDatabaseReady2{ 0x1402FF980, 0x1403E1840 };
	WEAK symbol<void()> Sys_ShowConsole{ 0x14038FA90, 0x1404D98B0 };

	/***************************************************************
	 * Variables
	 **************************************************************/

	WEAK symbol<int> keyCatchers{ 0x1413D5B00, 0x1417E168C };
	WEAK symbol<PlayerKeyState> playerKeys{ 0x1413BC5DC, 0x1417DA46C };

	WEAK symbol<CmdArgs> cmd_args{ 0x1492EC6F0, 0x1479ECB00 };
	WEAK symbol<CmdArgs> sv_cmd_args{ 0x1492EC7A0, 0x1479ECBB0 };
	WEAK symbol<cmd_function_s*> cmd_functions{ 0x1492EC848, 0x1479ECC58 };

	WEAK symbol<int> dvarCount{ 0x14A7BFF34, 0x14B32AA30 };
	WEAK symbol<dvar_t*> sortedDvars{ 0x14A7BFF50, 0x14B32AA50 };

	WEAK symbol<int> svs_numclients{ 0, 0x1496C4B0C };
	WEAK symbol<client_t> svs_clients{ 0, 0x1496C4B10 };

	namespace mp
	{
		WEAK symbol<gentity_s> g_entities{ 0, 0x144758C70 };
	}

	namespace sp
	{
		WEAK symbol<gentity_s> g_entities{ 0x143C26DC0, 0 };
	}
}
