#pragma once

namespace gsc
{
	extern void* func_table[0x1000];

	extern const game::dvar_t* developer_script;

	void scr_error(const char* error);
	void override_function(const std::string& name, game::BuiltinFunction func);
	void add_function(const std::string& name, game::BuiltinFunction function);
}
