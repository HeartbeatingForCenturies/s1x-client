#pragma once

namespace demonware
{

	class bdBandwidthTest final : public service
	{
	public:
		bdBandwidthTest();

	private:
		void unk204(service_server* server, uint8_t type, byte_buffer* buffer) const;
		void unk2(service_server* server, uint8_t type, byte_buffer* buffer) const;
	};

}
