#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/image.hpp>
#include <utils/io.hpp>

namespace images
{
	namespace
	{
		utils::hook::detour load_texture_hook;
	
		static_assert(sizeof(game::GfxImage) == 104);
		static_assert(offsetof(game::GfxImage, name) == (sizeof(game::GfxImage) - sizeof(void*)));
		static_assert(offsetof(game::GfxImage, loadDef) == 56);
		static_assert(offsetof(game::GfxImage, width) == 44);

		std::optional<std::string> load_image(game::GfxImage* image)
		{
			printf("Loading: %s\n", image->name);

			std::string data{};
			if(!utils::io::read_file(utils::string::va("s1x/images/%s.png", image->name), &data))
			{
				return {};
			}

			return {std::move(data)};
		}
	
		std::optional<utils::image> load_raw_image_from_file(game::GfxImage* image)
		{
			const auto image_file = load_image(image);
			if(!image_file)
			{
				return {};
			}

			return utils::image(*image_file);
		}

		void upload_texture(ID3D11Texture2D* texture, const utils::image& image)
		{
			D3D11_TEXTURE2D_DESC desc;
			ID3D11Device* device;
			ID3D11DeviceContext* context;

			texture->GetDesc(&desc);
			texture->GetDevice(&device);
			
			if(device)
			{
				device->GetImmediateContext(&context);
				device->Release();
			}

			auto _ = gsl::finally([&]()
			{
				if(context)
				{
					context->Release();
				}
			});

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
				}
			}
			else if (desc.Usage == D3D11_USAGE_DEFAULT)
			{
				D3D11_BOX box;
				box.front = 0;
				box.back = 1;
				box.left = 0;
				box.right = image.get_width();
				box.top = 0;
				box.bottom = image.get_height();

				context->UpdateSubresource(texture, 0, &box, image.get_buffer(), image.get_width() * 4, image.get_width() * image.get_height() * 4);
			}
		}

		bool load_custom_texture(game::GfxImage* image)
		{
			const auto raw_image = load_raw_image_from_file(image);
			if(!raw_image)
			{
				return false;
			}

			image->imageFormat |= 0x1000003;
			image->imageFormat &= ~0x2030000;

			game::Image_Setup(image, raw_image->get_width(), raw_image->get_height(), image->depth, image->numElements, image->imageFormat,
			                  DXGI_FORMAT_R8G8B8A8_UNORM, image->name, nullptr);

			upload_texture(image->textures.map, *raw_image);
			
			return true;
		}

		void load_texture_stub(game::GfxImageLoadDef** load_def, game::GfxImage* image)
		{
			if(!load_custom_texture(image))
			{
				load_texture_hook.invoke(load_def, image);
			}
		}
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
