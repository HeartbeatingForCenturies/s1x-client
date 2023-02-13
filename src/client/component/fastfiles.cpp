#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/dvars.hpp"

#include "fastfiles.hpp"
#include "command.hpp"
#include "console.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/concurrency.hpp>

namespace fastfiles
{
	static utils::concurrency::container<std::string> current_fastfile;

	namespace
	{
		utils::hook::detour db_try_load_x_file_internal_hook;
		utils::hook::detour db_find_x_asset_header_hook;

		void db_try_load_x_file_internal(const char* zone_name, const int flags)
		{
			console::info("Loading fastfile %s\n", zone_name);
			current_fastfile.access([&](std::string& fastfile)
			{
				fastfile = zone_name;
			});
			return db_try_load_x_file_internal_hook.invoke<void>(zone_name, flags);
		}

		void dump_gsc_script(const std::string& name, game::XAssetHeader header)
		{
			if (!dvars::g_dump_scripts->current.enabled)
			{
				return;
			}

			std::string buffer;
			buffer.append(header.scriptfile->name, std::strlen(header.scriptfile->name) + 1);
			buffer.append(reinterpret_cast<char*>(&header.scriptfile->compressedLen), sizeof(int));
			buffer.append(reinterpret_cast<char*>(&header.scriptfile->len), sizeof(int));
			buffer.append(reinterpret_cast<char*>(&header.scriptfile->bytecodeLen), sizeof(int));
			buffer.append(header.scriptfile->buffer, header.scriptfile->compressedLen);
			buffer.append(reinterpret_cast<char*>(header.scriptfile->bytecode), header.scriptfile->bytecodeLen);

			const auto out_name = std::format("gsc_dump/{}.gscbin", name);
			utils::io::write_file(out_name, buffer);

			console::info("Dumped %s\n", out_name.data());
		}

		game::XAssetHeader db_find_x_asset_header_stub(game::XAssetType type, const char* name, int allow_create_default)
		{
			const auto start = game::Sys_Milliseconds();
			const auto result = db_find_x_asset_header_hook.invoke<game::XAssetHeader>(type, name, allow_create_default);
			const auto diff = game::Sys_Milliseconds() - start;

			if (type == game::ASSET_TYPE_SCRIPTFILE)
			{
				dump_gsc_script(name, result);
			}

			if (diff > 100)
			{
				console::print(
					result.data == nullptr ? console::con_type_error : console::con_type_warning, "Waited %i msec for asset '%s' of type '%s'.\n",
					diff,
					name,
					game::g_assetNames[type]
				);
			}

			return result;
		}
	}

	std::string get_current_fastfile()
	{
		auto fastfile_copy = current_fastfile.access<std::string>([&](std::string& fastfile)
		{
			return fastfile;
		});

		return fastfile_copy;
	}

	constexpr int get_asset_type_size(const game::XAssetType type)
	{
		constexpr int asset_type_sizes[] =
		{
			96, 88, 128, 56, 40, 216, 56, 680,
			480, 32, 32, 32, 32, 32, 352, 1456,
			104, 32, 24, 152, 152, 152, 16, 64,
			640, 40, 16, 408, 24, 288, 176, 2800,
			48, -1, 40, 24, 200, 88, 16, 120,
			3560, 32, 64, 16, 16, -1, -1, -1,
			-1, 24, 40, 24, 40, 24, 128, 2256,
			136, 32, 72, 24, 64, 88, 48, 32,
			96, 152, 64, 32,
		};

		return asset_type_sizes[type];
	}

	template <game::XAssetType Type, size_t Size>
	char* reallocate_asset_pool()
	{
		constexpr auto element_size = get_asset_type_size(Type);
		static char new_pool[element_size * Size] = {0};
		assert(get_asset_type_size(Type) == game::DB_GetXAssetTypeSize(Type));

		std::memmove(new_pool, game::DB_XAssetPool[Type], game::g_poolSize[Type] * element_size);

		game::DB_XAssetPool[Type] = new_pool;
		game::g_poolSize[Type] = Size;

		return new_pool;
	}

	void enum_assets(const game::XAssetType type, const std::function<void(game::XAssetHeader)>& callback, const bool include_override)
	{
		game::DB_EnumXAssets_Internal(type, static_cast<void(*)(game::XAssetHeader, void*)>([](game::XAssetHeader header, void* data)
		{
			const auto& cb = *static_cast<const std::function<void(game::XAssetHeader)>*>(data);
			cb(header);
		}), &callback, include_override);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			db_try_load_x_file_internal_hook.create(
				SELECT_VALUE(0x1401816F0, 0x1402741C0), &db_try_load_x_file_internal);

			db_find_x_asset_header_hook.create(game::DB_FindXAssetHeader, db_find_x_asset_header_stub);
			dvars::g_dump_scripts = game::Dvar_RegisterBool("g_dumpScripts", false, game::DVAR_FLAG_NONE, "Dump GSC scripts to binary format");

			command::add("loadzone", [](const command::params& params)
			{
				if (params.size() < 2)
				{
					console::info("usage: loadzone <zone>\n");
					return;
				}

				game::XZoneInfo info{};
				info.name = params.get(1);
				info.allocFlags = 1;
				info.freeFlags = 0;
				game::DB_LoadXAssets(&info, 1u, game::DBSyncMode::DB_LOAD_SYNC);
			});

			command::add("g_poolSizes", []()
			{
				for (auto i = 0; i < game::ASSET_TYPE_COUNT; i++)
				{
					console::info("g_poolSize[%i]: %i // %s\n", i, game::g_poolSize[i], game::g_assetNames[i]);
				}
			});

			reallocate_asset_pool<game::ASSET_TYPE_FONT, 48>();

			if (!game::environment::is_sp())
			{
				const auto* xmodel_pool = reallocate_asset_pool<game::ASSET_TYPE_XMODEL, 8832>();
				utils::hook::inject(0x14026FD63, xmodel_pool + 8);
				utils::hook::inject(0x14026FDB3, xmodel_pool + 8);
				utils::hook::inject(0x14026FFAC, xmodel_pool + 8);
				utils::hook::inject(0x14027463C, xmodel_pool + 8);
				utils::hook::inject(0x140274689, xmodel_pool + 8);
			}
		}
	};
}

REGISTER_COMPONENT(fastfiles::component)
