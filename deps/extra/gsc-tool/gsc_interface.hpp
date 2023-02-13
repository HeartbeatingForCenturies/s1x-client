#pragma once
#undef ERROR
#undef IN
#undef TRUE
#undef FALSE

#undef far

#include <stdinc.hpp>
#include <s1/s1_pc.hpp>

namespace gsc
{
	extern std::unique_ptr<xsk::gsc::s1_pc::context> cxt;
}
