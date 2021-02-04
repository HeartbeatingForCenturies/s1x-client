#pragma once

namespace demonware
{
	class bdMarketing final : public service
	{
	public:
		bdMarketing();

	private:
		void unk3(service_server* server, byte_buffer* buffer) const;
	};
}
