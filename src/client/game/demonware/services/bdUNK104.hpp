#pragma once

namespace demonware
{
	class bdUNK104 final : public service
	{
	public:
		bdUNK104();

	private:
		void unk1(service_server* server, byte_buffer* buffer) const;
	};
}
