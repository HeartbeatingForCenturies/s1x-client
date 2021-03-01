#pragma once
#include <utils/cryptography.hpp>
#include "new/tcp_server.hpp"

using server_ptr = std::shared_ptr<tcp_server>;

namespace demonware
{
#pragma pack(push, 1)
	struct auth_ticket_t
	{
		unsigned int m_magicNumber;
		char m_type;
		unsigned int m_titleID;
		unsigned int m_timeIssued;
		unsigned int m_timeExpires;
		unsigned __int64 m_licenseID;
		unsigned __int64 m_userID;
		char m_username[64];
		char m_sessionKey[24];
		char m_usingHashMagicNumber[3];
		char m_hash[4];
	};
#pragma pack(pop)

	class server_auth3 : public tcp_server
	{
	public:
		using tcp_server::tcp_server;

	private:
		void send_reply(reply* data);
		void handle(const std::string& packet) override;
	};
}
