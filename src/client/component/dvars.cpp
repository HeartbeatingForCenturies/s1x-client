#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "dvars.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace dvars
{
	namespace override
	{
		class dvar_bool
		{
		public:
			const char* name;
			bool value;
			unsigned int flags;
			const char* description;
		};

		class dvar_float
		{
		public:
			const char* name;
			float value;
			float min;
			float max;
			unsigned int flags;
			const char* description;
		};

		class dvar_int
		{
		public:
			const char* name;
			int value;
			int min;
			int max;
			unsigned int flags;
			const char* description;
		};

		class dvar_string
		{
		public:
			const char* name;
			const char* value;
			unsigned int flags;
			const char* description;
		};

		class dvar_setstring
		{
		public:
			const char* dvar_name;
			const char* string;
		};

		static std::vector<dvar_bool> bool_overrides;
		static std::vector<dvar_float> float_overrides;
		static std::vector<dvar_int> int_overrides;
		static std::vector<dvar_string> string_overrides;

		static std::vector<dvar_setstring> set_string_overrides;

		void Dvar_RegisterBool(const char* name, bool value, unsigned int flags, const char* description)
		{
			dvar_bool values;
			values.name = name;
			values.value = value;
			values.flags = flags;
			values.description = description;
			bool_overrides.push_back(std::move(values));
		}

		void Dvar_RegisterFloat(const char* name, float value, float min, float max, unsigned int flags, const char* description)
		{
			dvar_float values;
			values.name = name;
			values.value = value;
			values.min = min;
			values.max = max;
			values.flags = flags;
			values.description = description;
			float_overrides.push_back(std::move(values));
		}

		void Dvar_RegisterInt(const char* name, int value, int min, int max, unsigned int flags, const char* description)
		{
			dvar_int values;
			values.name = name;
			values.value = value;
			values.min = min;
			values.max = max;
			values.flags = flags;
			values.description = description;
			int_overrides.push_back(std::move(values));
		}

		void Dvar_RegisterString(const char* name, const char* value, unsigned int flags, const char* description)
		{
			dvar_string values;
			values.name = name;
			values.value = value;
			values.flags = flags;
			values.description = description;
			string_overrides.push_back(std::move(values));
		}

		void Dvar_SetString(const char* dvar_name, const char* string)
		{
			dvar_setstring setstring;
			setstring.dvar_name = dvar_name;
			setstring.string = string;
			set_string_overrides.push_back(std::move(setstring));
		}
	}

	utils::hook::detour dvar_register_bool_hook;
	utils::hook::detour dvar_register_float_hook;
	utils::hook::detour dvar_register_int_hook;
	utils::hook::detour dvar_register_string_hook;

	utils::hook::detour dvar_set_string_hook;

	game::dvar_t* dvar_register_bool(const char* name, bool value, unsigned int flags, const char* description)
	{
		for (auto i = 0; i < override::bool_overrides.size(); i++)
		{
			if (!strcmp(name, override::bool_overrides[i].name))
			{
				auto* dv = &override::bool_overrides[i];
				value = dv->value;
				flags = dv->flags;
				description = dv->description;
				break;
			}
		}

		return dvar_register_bool_hook.invoke<game::dvar_t*>(name, value, flags, description);
	}

	game::dvar_t* dvar_register_float(const char* name, float value, float min, float max, unsigned int flags, const char* description)
	{
		for (auto i = 0; i < override::float_overrides.size(); i++)
		{
			if (!strcmp(name, override::float_overrides[i].name))
			{
				auto* dv = &override::float_overrides[i];
				value = dv->value;
				min = dv->min;
				max = dv->max;
				flags = dv->flags;
				description = dv->description;
				break;
			}
		}

		return dvar_register_float_hook.invoke<game::dvar_t*>(name, value, min, max, flags, description);
	}

	game::dvar_t* dvar_register_int(const char* name, int value, int min, int max, unsigned int flags, const char* description)
	{
		for (auto i = 0; i < override::int_overrides.size(); i++)
		{
			if (!strcmp(name, override::int_overrides[i].name))
			{
				auto* dv = &override::int_overrides[i];
				value = dv->value;
				min = dv->min;
				max = dv->max;
				flags = dv->flags;
				description = dv->description;
				break;
			}
		}

		return dvar_register_int_hook.invoke<game::dvar_t*>(name, value, min, max, flags, description);
	}

	game::dvar_t* dvar_register_string(const char* name, const char* value, unsigned int flags, const char* description)
	{
		for (auto i = 0; i < override::string_overrides.size(); i++)
		{
			if (!strcmp(name, override::string_overrides[i].name))
			{
				auto* dv = &override::string_overrides[i];
				value = dv->value;
				flags = dv->flags;
				description = dv->description;
				break;
			}
		}

		return dvar_register_string_hook.invoke<game::dvar_t*>(name, value, flags, description);
	}

	bool dvar_set_string(game::dvar_t* dvar, const char* string)
	{
		for (auto i = 0; i < override::set_string_overrides.size(); i++)
		{
			if (!strcmp(dvar->name, override::set_string_overrides[i].dvar_name))
			{
				string = override::set_string_overrides[i].string;
				break;
			}
		}

		return dvar_set_string_hook.invoke<bool>(dvar, string);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			dvar_register_bool_hook.create(SELECT_VALUE(0x140371850, 0x1404C0BE0), &dvar_register_bool);
			dvar_register_float_hook.create(SELECT_VALUE(0x140371C20, 0x1404C0FB0), &dvar_register_float);
			dvar_register_int_hook.create(SELECT_VALUE(0x140371CF0, 0x1404C1080), &dvar_register_int);
			dvar_register_string_hook.create(SELECT_VALUE(0x140372050, 0x1404C1450), &dvar_register_string);

			dvar_set_string_hook.create(SELECT_VALUE(0x140373DE0, 0x1404C3610), &dvar_set_string);
		}
	};
}

REGISTER_COMPONENT(dvars::component)
