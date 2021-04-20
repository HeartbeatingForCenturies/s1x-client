#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "fastfiles.hpp"

#include "command.hpp"
#include "console.hpp"

#include <utils/hook.hpp>
#include <utils/concurrency.hpp>

namespace fastfiles
{
	static utils::concurrency::container<std::string> current_fastfile;

	namespace
	{
		utils::hook::detour db_try_load_x_file_internal_hook;

		void db_try_load_x_file_internal(const char* zone_name, const int flags)
		{
			console::info("Loading fastfile %s\n", zone_name);
			current_fastfile.access([&](std::string& fastfile)
			{
				fastfile = zone_name;
			});
			return db_try_load_x_file_internal_hook.invoke<void>(zone_name, flags);
		}
	}

	std::string get_current_fastfile()
	{
		std::string fastfile_copy;
		current_fastfile.access([&](std::string& fastfile)
		{
			fastfile_copy = fastfile;
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

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			db_try_load_x_file_internal_hook.create(
				SELECT_VALUE(0x1401816F0, 0x1402741C0), &db_try_load_x_file_internal);

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
