#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "images.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/image.hpp>
#include <utils/io.hpp>
#include <utils/concurrency.hpp>

namespace images
{
	namespace
	{
		utils::hook::detour load_texture_hook;
		utils::concurrency::container<std::unordered_map<std::string, std::string>> overriden_textures;
	
		static_assert(sizeof(game::GfxImage) == 104);
		static_assert(offsetof(game::GfxImage, name) == (sizeof(game::GfxImage) - sizeof(void*)));
		static_assert(offsetof(game::GfxImage, loadDef) == 56);
		static_assert(offsetof(game::GfxImage, width) == 44);

		std::optional<std::string> load_image(game::GfxImage* image)
		{
			std::string data{};
			overriden_textures.access([&](const std::unordered_map<std::string, std::string>& textures)
			{
				if (const auto i = textures.find(image->name); i != textures.end())
				{
					data = i->second;
				}
			});
			
			if (data.empty() && !utils::io::read_file(utils::string::va("s1x/images/%s.png", image->name), &data))
			{
				return {};
			}

			return {std::move(data)};
		}
	
		std::optional<utils::image> load_raw_image_from_file(game::GfxImage* image)
		{
			const auto image_file = load_image(image);
			if (!image_file)
			{
				return {};
			}

			return utils::image(*image_file);
		}

		bool load_custom_texture(game::GfxImage* image)
		{
			auto raw_image = load_raw_image_from_file(image);
			if (!raw_image)
			{
				return false;
			}

			image->imageFormat |= 0x1000003;
			image->imageFormat &= ~0x2030000;

			D3D11_SUBRESOURCE_DATA data{};
			data.SysMemPitch = raw_image->get_width() * 4;
			data.SysMemSlicePitch = data.SysMemPitch * raw_image->get_height();
			data.pSysMem = raw_image->get_buffer();

			game::Image_Setup(image, raw_image->get_width(), raw_image->get_height(), image->depth, image->numElements, image->imageFormat,
			                  DXGI_FORMAT_R8G8B8A8_UNORM, image->name, &data);

			return true;
		}

		void load_texture_stub(game::GfxImageLoadDef** load_def, game::GfxImage* image)
		{
#if defined(DEV_BUILD) && defined(DEBUG)
			printf("Loading: %s\n", image->name);
#endif

			try
			{
				if (load_custom_texture(image))
				{
					return;
				}
			}
			catch(std::exception&)
			{
			}

			load_texture_hook.invoke(load_def, image);
		}
	}

	void override_texture(std::string name, std::string data)
	{
		overriden_textures.access([&](std::unordered_map<std::string, std::string>& textures)
		{
			textures[std::move(name)] = std::move(data);
		});
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			load_texture_hook.create(SELECT_VALUE(0x140484970, 0x1405A21F0), load_texture_stub);
		}
	};
}

REGISTER_COMPONENT(images::component)
