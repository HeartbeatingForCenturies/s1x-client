#pragma once

namespace demonware
{

	class bdProfiles final : public service
	{
	public:
		bdProfiles();

	private:
		void unk3(service_server* server, uint8_t type, byte_buffer* buffer) const;
	};

}
