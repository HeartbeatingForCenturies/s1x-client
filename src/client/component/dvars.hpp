#pragma once

namespace dvars
{
	namespace disable
	{
		void Dvar_SetInt(const char* dvar_name);
		void Dvar_SetString(const char* dvar_name);
	}

	namespace override
	{
		void Dvar_RegisterBool(const char* name, bool value, unsigned int flags, const char* description = "");
		void Dvar_RegisterFloat(const char* name, float value, float min, float max, unsigned int flags, const char* description = "");
		void Dvar_RegisterInt(const char* name, int value, int min, int max, unsigned int flags, const char* description = "");
		void Dvar_RegisterString(const char* name, const char* value, unsigned int flags, const char* description = "");

		void Dvar_SetString(const char* dvar_name, const char* string);
	}
}
