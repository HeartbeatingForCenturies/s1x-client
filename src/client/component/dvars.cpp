#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "dvars.hpp"

#include "game/game.hpp"

#include <utils/hook.hpp>

namespace dvars
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

	class dvar_setint
	{
	public:
		const char* name;
		int integer;
	};

	class dvar_setstring
	{
	public:
		const char* name;
		const char* string;
	};

	namespace
	{
		template<typename T>
		T* find_dvar(std::vector<T>* vec, const char* name)
		{
			for (auto i = 0; i < vec->size(); i++)
			{
				if (!strcmp(name, vec->at(i).name))
				{
					return &vec->at(i);
				}
			}
			return nullptr;
		}
	}

	namespace disable
	{
		static std::vector<dvar_setint> set_int_disables;
		static std::vector<dvar_setstring> set_string_disables;

		void Dvar_SetInt(const char* dvar_name)
		{
			dvar_setint values;
			values.name = dvar_name;
			set_int_disables.push_back(std::move(values));
		}

		void Dvar_SetString(const char* dvar_name)
		{
			dvar_setstring values;
			values.name = dvar_name;
			set_string_disables.push_back(std::move(values));
		}
	}

	namespace override
	{
		static std::vector<dvar_bool> bool_overrides;
		static std::vector<dvar_float> float_overrides;
		static std::vector<dvar_int> int_overrides;
		static std::vector<dvar_string> string_overrides;

		static std::vector<dvar_setint> set_int_overrides;
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

		void Dvar_SetInt(const char* name, int integer)
		{
			dvar_setint values;
			values.name = name;
			values.integer = integer;
			set_int_overrides.push_back(std::move(values));
		}

		void Dvar_SetString(const char* name, const char* string)
		{
			dvar_setstring values;
			values.name = name;
			values.string = string;
			set_string_overrides.push_back(std::move(values));
		}
	}

	utils::hook::detour dvar_register_bool_hook;
	utils::hook::detour dvar_register_float_hook;
	utils::hook::detour dvar_register_int_hook;
	utils::hook::detour dvar_register_string_hook;

	utils::hook::detour dvar_set_int_hook;
	utils::hook::detour dvar_set_string_hook;

	game::dvar_t* dvar_register_bool(const char* name, bool value, unsigned int flags, const char* description)
	{
		auto* var = find_dvar(&override::bool_overrides, name);

		if (var)
		{
			value = var->value;
			flags = var->flags;
			description = var->description;
		}

		return dvar_register_bool_hook.invoke<game::dvar_t*>(name, value, flags, description);
	}

	game::dvar_t* dvar_register_float(const char* name, float value, float min, float max, unsigned int flags, const char* description)
	{
		auto* var = find_dvar(&override::float_overrides, name);
		if (var)
		{
			value = var->value;
			min = var->min;
			max = var->max;
			flags = var->flags;
			description = var->description;
		}

		return dvar_register_float_hook.invoke<game::dvar_t*>(name, value, min, max, flags, description);
	}

	game::dvar_t* dvar_register_int(const char* name, int value, int min, int max, unsigned int flags, const char* description)
	{
		auto* var = find_dvar(&override::int_overrides, name);
		if (var)
		{
			value = var->value;
			min = var->min;
			max = var->max;
			flags = var->flags;
			description = var->description;
		}

		return dvar_register_int_hook.invoke<game::dvar_t*>(name, value, min, max, flags, description);
	}

	game::dvar_t* dvar_register_string(const char* name, const char* value, unsigned int flags, const char* description)
	{
		auto* var = find_dvar(&override::string_overrides, name);
		if (var)
		{
			value = var->value;
			flags = var->flags;
			description = var->description;
		}

		return dvar_register_string_hook.invoke<game::dvar_t*>(name, value, flags, description);
	}

	void dvar_set_int(game::dvar_t* dvar, int integer)
	{
		auto* var = find_dvar(&disable::set_int_disables, dvar->name);
		if (var)
		{
			return;
		}

		var = find_dvar(&override::set_int_overrides, dvar->name);
		if (var)
		{
			integer = var->integer;
		}

		return dvar_set_int_hook.invoke<void>(dvar, integer);
	}

	void dvar_set_string(game::dvar_t* dvar, const char* string)
	{
		auto* var = find_dvar(&disable::set_string_disables, dvar->name);
		if (var)
		{
			return;
		}

		var = find_dvar(&override::set_string_overrides, dvar->name);
		if (var)
		{
			string = var->string;
		}

		return dvar_set_string_hook.invoke<void>(dvar, string);
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

			dvar_set_int_hook.create(SELECT_VALUE(0x1403738D0, 0x1404C2F40), &dvar_set_int);
			dvar_set_string_hook.create(SELECT_VALUE(0x140373DE0, 0x1404C3610), &dvar_set_string);
		}
	};
}

REGISTER_COMPONENT(dvars::component)
