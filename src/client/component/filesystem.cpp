#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "dvars.hpp"

#include "filesystem.hpp"
#include "game_module.hpp"
#include "console.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

namespace filesystem
{
	namespace
	{
		bool initialized = false;

		bool custom_path_registered = false;

		std::deque<std::filesystem::path>& get_search_paths_internal()
		{
			static std::deque<std::filesystem::path> search_paths{};
			return search_paths;
		}

		std::string get_binary_directory()
		{
			const auto dir = game_module::get_host_module().get_folder();
			return utils::string::replace(dir, "/", "\\");
		}

		void register_custom_path_stub(const char* path, const char* dir)
		{
			if (!custom_path_registered)
			{
				custom_path_registered = true;

				const auto launcher_dir = get_binary_directory();
				game::FS_AddLocalizedGameDirectory(launcher_dir.data(), "data");
			}

			game::FS_AddLocalizedGameDirectory(path, dir);
		}

		void fs_startup_stub(const char* gamename)
		{
			console::info("[FS] Startup\n");

			custom_path_registered = false;

			game::FS_Startup(gamename);
		}

		bool can_insert_path(const std::filesystem::path& path)
		{
			const auto& paths = get_search_paths_internal();
			return std::ranges::none_of(paths.cbegin(), paths.cend(), [path](const auto& elem)
			{
				return elem == path;
			});
		}

		void startup()
		{
			register_path("s1x");
			register_path(get_binary_directory() + "\\data");

			// game's search paths
			register_path("devraw");
			register_path("devraw_shared");
			register_path("raw_shared");
			register_path("raw");
			register_path("main");
		}

		void check_for_startup()
		{
			if (!initialized)
			{
				initialized = true;
				startup();
			}
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

	std::string read_file(const std::string& path)
	{
		check_for_startup();

		for (const auto& search_path : get_search_paths_internal())
		{
			const auto path_ = search_path / path;
			if (utils::io::file_exists(path_.generic_string()))
			{
				return utils::io::read_file(path_.generic_string());
			}
		}

		return {};
	}

	bool read_file(const std::string& path, std::string* data, std::string* real_path)
	{
		check_for_startup();

		for (const auto& search_path : get_search_paths_internal())
		{
			const auto path_ = search_path / path;
			if (utils::io::read_file(path_.generic_string(), data))
			{
				if (real_path != nullptr)
				{
					*real_path = path_.generic_string();
				}

				return true;
			}
		}

		return false;
	}

	bool find_file(const std::string& path, std::string* real_path)
	{
		check_for_startup();

		for (const auto& search_path : get_search_paths_internal())
		{
			const auto path_ = search_path / path;
			if (utils::io::file_exists(path_.generic_string()))
			{
				*real_path = path_.generic_string();
				return true;
			}
		}

		return false;
	}

	bool exists(const std::string& path)
	{
		check_for_startup();

		for (const auto& search_path : get_search_paths_internal())
		{
			const auto path_ = search_path / path;
			if (utils::io::file_exists(path_.generic_string()))
			{
				return true;
			}
		}

		return false;
	}

	void register_path(const std::filesystem::path& path)
	{
		if (can_insert_path(path))
		{
			console::info("[FS] Registering path '%s'\n", path.generic_string().data());
			get_search_paths_internal().push_front(path);
		}
	}

	void unregister_path(const std::filesystem::path& path)
	{
		if (!initialized)
		{
			return;
		}

		auto& search_paths = get_search_paths_internal();
		for (auto i = search_paths.begin(); i != search_paths.end();)
		{
			if (*i == path)
			{
				console::info("[FS] Unregistering path '%s'\n", path.generic_string().data());
				i = search_paths.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	std::vector<std::string> get_search_paths()
	{
		std::vector<std::string> paths{};

		for (const auto& path : get_search_paths_internal())
		{
			paths.push_back(path.generic_string());
		}

		return paths;
	}

	std::vector<std::string> get_search_paths_rev()
	{
		std::vector<std::string> paths{};
		const auto& search_paths = get_search_paths_internal();

		for (auto i = search_paths.rbegin(); i != search_paths.rend(); ++i)
		{
			paths.push_back(i->generic_string());
		}

		return paths;
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// Set fs_basegame
			dvars::override::register_string("fs_basegame", "s1x", game::DVAR_FLAG_WRITE);

			if (game::environment::is_sp())
			{
				utils::hook::call(0x140360A74, fs_startup_stub);

				utils::hook::call(0x140361FE0, register_custom_path_stub);
				utils::hook::call(0x140362000, register_custom_path_stub);
				utils::hook::call(0x14036203F, register_custom_path_stub);
			}
			else
			{
				utils::hook::call(0x1404AE192, fs_startup_stub);
				utils::hook::call(0x1404AE5C3, fs_startup_stub);

				utils::hook::call(0x1404AEFD0, register_custom_path_stub);
				utils::hook::call(0x1404AEFF0, register_custom_path_stub);
				utils::hook::call(0x1404AF02F, register_custom_path_stub);
			}
		}
	};
}

REGISTER_COMPONENT(filesystem::component)
