#include <std_include.hpp>
#include "types.hpp"
#include "execution.hpp"
#include "stack_isolation.hpp"

namespace ui_scripting
{
	/***************************************************************
	 * Lightuserdata
	 **************************************************************/

	lightuserdata::lightuserdata(void* ptr_)
		: ptr(ptr_)
	{
	}

	/***************************************************************
	 * Userdata
	 **************************************************************/

	userdata::userdata(void* ptr_)
		: ptr(ptr_)
	{
	}

	void userdata::set(const script_value& key, const script_value& value) const
	{
		set_field(*this, key, value);
	}

	script_value userdata::get(const script_value& key) const
	{
		return get_field(*this, key);
	}

	/***************************************************************
	 * Table
	 **************************************************************/

	table::table()
	{
		const auto state = *game::hks::lua_state;
		this->ptr = game::hks::Hashtable_Create(state, 0, 0);
	}

	table::table(game::hks::HashTable* ptr_)
		: ptr(ptr_)
	{
	}

	void table::set(const script_value& key, const script_value& value) const
	{
		set_field(*this, key, value);
	}

	script_value table::get(const script_value& key) const
	{
		return get_field(*this, key);
	}

	/***************************************************************
	 * Function
	 **************************************************************/

	function::function(game::hks::cclosure* ptr_, game::hks::HksObjectType type_)
		: ptr(ptr_)
		, type(type_)
	{
		this->add();
	}

	function::function(const function& other) : function(other.ptr, other.type)
	{
		this->ref = other.ref;
	}

	function::function(function&& other) noexcept
	{
		this->ptr = other.ptr;
		this->type = other.type;
		this->ref = other.ref;
		other.ref = 0;
	}

	function::~function()
	{
		this->release();
	}

	function& function::operator=(const function& other)
	{
		if (&other != this)
		{
			this->release();
			this->ptr = other.ptr;
			this->type = other.type;
			this->ref = other.ref;
			this->add();
		}

		return *this;
	}

	function& function::operator=(function&& other) noexcept
	{
		if (&other != this)
		{
			this->release();
			this->ptr = other.ptr;
			this->type = other.type;
			this->ref = other.ref;
			other.ref = 0;
		}

		return *this;
	}

	void function::add()
	{
		game::hks::HksObject value{};
		value.v.cClosure = this->ptr;
		value.t = this->type;

		stack_isolation _;
		push_value(value);

		this->ref = game::hks::hksi_luaL_ref(*game::hks::lua_state, -10000);
	}

	void function::release()
	{
		if (this->ref)
		{
			game::hks::hksi_luaL_unref(*game::hks::lua_state, -10000, this->ref);
		}
	}

	arguments function::call(const arguments& arguments) const
	{
		return call_script_function(*this, arguments);
	}
}