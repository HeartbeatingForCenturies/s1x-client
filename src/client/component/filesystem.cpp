#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "filesystem.hpp"
#include "game_module.hpp"

#include "game/game.hpp"
#include "dvars.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace filesystem
{
	namespace
	{
		std::string get_binary_directory()
		{
			const auto dir = game_module::get_host_module().get_folder();
			return utils::string::replace(dir, "/", "\\");
		}

		void fs_startup_stub(const char* gamename)
		{
			game::FS_Startup(gamename);

			const auto launcher_dir = get_binary_directory();
			game::FS_AddLocalizedGameDirectory(launcher_dir.data(), "data");
		}
	}

	file::file(std::string name)
		: name_(std::move(name))
	{
		char* buffer{};
		const auto size = game::FS_ReadFile(this->name_.data(), &buffer);

		if (size >= 0 && buffer)
		{
			this->valid_ = true;
			this->buffer_.append(buffer, size);
			game::FS_FreeFile(buffer);
		}
	}

	bool file::exists() const
	{
		return this->valid_;
	}

	const std::string& file::get_buffer() const
	{
		return this->buffer_;
	}

	const std::string& file::get_name() const
	{
		return this->name_;
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// Set fs_basegame
			dvars::override::Dvar_RegisterString("fs_basegame", "s1x", game::DVAR_FLAG_WRITE);

			if (game::environment::is_sp())
			{
				utils::hook::call(0x140360A74, fs_startup_stub);
			}
			else
			{
				utils::hook::call(0x1404AE192, fs_startup_stub);
				utils::hook::call(0x1404AE5C3, fs_startup_stub);
			}
		}
	};
}

REGISTER_COMPONENT(filesystem::component)
