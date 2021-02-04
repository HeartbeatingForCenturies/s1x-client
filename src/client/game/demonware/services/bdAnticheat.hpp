#pragma once

namespace demonware
{

	class bdAnticheat final : public service
	{
	public:
		bdAnticheat();

	private:
		void unk2(service_server* server, uint8_t type, byte_buffer* buffer) const;
		void report_console_details(service_server* server, uint8_t type, byte_buffer* buffer) const;
	};

}
