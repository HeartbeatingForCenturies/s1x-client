#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/io.hpp>
#include <utils/nt.hpp>
#include <utils/http.hpp>
#include <utils/toast.hpp>
#include <utils/binary_resource.hpp>

#include <version.hpp>

#define APPVEYOR_ARTIFACT_BASE "https://master.xlabs.dev:444"
#define APPVEYOR_PROJECT "s1x"
#define APPVEYOR_BRANCH GIT_BRANCH

#ifdef DEBUG
#define APPVEYOR_CONFIGURATION "Debug"
#else
#define APPVEYOR_CONFIGURATION "Release"
#endif

#define APPVEYOR_ARTIFACT_URL(artifact) (APPVEYOR_ARTIFACT_BASE "/" APPVEYOR_PROJECT "/" APPVEYOR_BRANCH "/" APPVEYOR_CONFIGURATION "/" artifact)

#define APPVEYOR_S1X_EXE    APPVEYOR_ARTIFACT_URL("build/bin/x64/" APPVEYOR_CONFIGURATION "/" APPVEYOR_PROJECT ".exe")
#define APPVEYOR_VERSION_TXT APPVEYOR_ARTIFACT_URL("build/version.txt")

namespace updater
{
	namespace
	{
		utils::binary_resource s1x_icon(ICON_IMAGE, "s1x-icon.png");

		bool is_update_available()
		{
			const auto version = utils::http::get_data(APPVEYOR_VERSION_TXT);
			return version && !version->empty() && version != GIT_HASH;
		}

		void perform_update(const std::string& target)
		{
			const auto binary = utils::http::get_data(APPVEYOR_S1X_EXE);
			utils::io::write_file(target, *binary);
		}

		void delete_old_file(const std::string& file)
		{
			// Wait for other process to die
			for (auto i = 0; i < 4; ++i)
			{
				utils::io::remove_file(file);

				if (utils::io::file_exists(file))
				{
					std::this_thread::sleep_for(2s);
				}
				else
				{
					break;
				}
			}
		}

		bool try_updating()
		{
			const utils::nt::library self;
			const auto self_file = self.get_path();
			const auto dead_file = self_file + ".old";

			delete_old_file(dead_file);

			if (!is_update_available())
			{
				return false;
			}

			const auto info = utils::toast::show("Updating S1x", "Please wait...", s1x_icon.get_extracted_file());

			utils::io::move_file(self_file, dead_file);
			perform_update(self_file);
			utils::nt::relaunch_self();

			info.hide();
			return true;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			if (try_updating())
			{
				component_loader::trigger_premature_shutdown();
			}
		}
	};
}

#if defined(CI) && !defined(DEBUG)
REGISTER_COMPONENT(updater::component)
#endif
