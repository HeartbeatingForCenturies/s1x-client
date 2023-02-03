#include "gsc_interface.hpp"

namespace gsc
{
	const std::unique_ptr<xsk::gsc::s1_pc::context> cxt = std::make_unique<xsk::gsc::s1_pc::context>();
}
