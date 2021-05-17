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

		bool upload_texture(CComPtr<ID3D11Texture2D> texture, const utils::image& image)
		{
			D3D11_TEXTURE2D_DESC desc;
			CComPtr<ID3D11Device> device;
			CComPtr<ID3D11DeviceContext> context;

			texture->GetDesc(&desc);
			texture->GetDevice(&device);
			if (!device) return false;

			device->GetImmediateContext(&context);
			if (!context) return false;

			if (desc.Usage == D3D11_USAGE_DYNAMIC && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) == D3D11_CPU_ACCESS_WRITE)
			{
				D3D11_MAPPED_SUBRESOURCE texmap;
				if (SUCCEEDED(context->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &texmap)))
				{
					for (int row = 0; row < image.get_height(); ++row)
					{
						std::memcpy(PBYTE(texmap.pData) + row * texmap.RowPitch, PBYTE(image.get_buffer()) + (4 * image.get_width()) * row, std::min(static_cast<int>(texmap.RowPitch), 4 * image.get_width()));
					}

					context->Unmap(texture, 0);
					return true;
				}
			}

			return false;
		}

		void schedule_texture_upload(ID3D11Texture2D* texture, utils::image&& image)
		{
			scheduler::once([texture, img{std::move(image)}]
			{
				upload_texture(texture, img);
			}, scheduler::renderer);
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

			game::Image_Setup(image, raw_image->get_width(), raw_image->get_height(), image->depth, image->numElements, image->imageFormat,
			                  DXGI_FORMAT_R8G8B8A8_UNORM, image->name, nullptr);

			schedule_texture_upload(image->textures.map, std::move(*raw_image));
			
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
			if (!game::environment::is_mp()) return;

			load_texture_hook.create(0x1405A21F0, load_texture_stub);
		}
	};
}

REGISTER_COMPONENT(images::component)
