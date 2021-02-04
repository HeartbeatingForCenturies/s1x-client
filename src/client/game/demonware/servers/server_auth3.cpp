#include <std_include.hpp>
#include "../demonware.hpp"
#include <utils/json.hpp>
using json = nlohmann::json;

namespace demonware
{
	int server_auth3::recv(const char* buf, const int len)
	{
		if (len <= 0) return -1;
		std::lock_guard<std::recursive_mutex> _(this->mutex_);
		this->incoming_queue_.push(std::string(buf, len));

		return len;
	}

	int server_auth3::send(char* buf, int len)
	{
		if (len > 0 && !this->outgoing_queue_.empty())
		{
			std::lock_guard<std::recursive_mutex> _(this->mutex_);

			len = std::min(len, static_cast<int>(this->outgoing_queue_.size()));
			for (auto i = 0; i < len; ++i)
			{
				buf[i] = this->outgoing_queue_.front();
				this->outgoing_queue_.pop();
			}

			return len;
		}

		return SOCKET_ERROR;
	}

	void server_auth3::send_reply(reply* data)
	{
		if (!data) return;

		std::lock_guard _(this->mutex_);

		const auto buffer = data->data();
		for (auto& byte : buffer)
		{
			this->outgoing_queue_.push(byte);
		}
	}

	void server_auth3::frame()
	{
		if (!this->incoming_queue_.empty())
		{
			std::lock_guard _(this->mutex_);
			const auto packet = this->incoming_queue_.front();
			this->incoming_queue_.pop();

			this->dispatch(packet);
		}
	}

	bool server_auth3::pending_data()
	{
		std::lock_guard _(this->mutex_);
		return !this->outgoing_queue_.empty();
	}

	void server_auth3::dispatch(const std::string& packet)
	{
		if (packet.starts_with("POST /auth/"))
		{
#ifdef DEBUG
			printf("[demonware]: [auth]: user requested authentication.\n");
#endif
			return;
		}
		else
		{
			unsigned int title_id = 0;
			unsigned int iv_seed = 0;
			std::string identity = "";
			std::string token = "";

			json j = json::parse(packet);

			if (j.contains("title_id") && j["title_id"].is_string())
				title_id = std::stoul(j["title_id"].get<std::string>());

			if (j.contains("iv_seed") && j["iv_seed"].is_string())
				iv_seed = std::stoul(j["iv_seed"].get<std::string>());

			if (j.contains("extra_data") && j["extra_data"].is_string())
			{
				json extra_data = json::parse(j["extra_data"].get<std::string>());

				if (extra_data.contains("token") && extra_data["token"].is_string())
				{
					std::string token_b64 = extra_data["token"].get<std::string>();
					token = utils::cryptography::base64::decode(token_b64);
				}
			}

#ifdef DEBUG
			printf("[demonware]: [auth]: authenticating user %s\n", std::string(&token.data()[64]).data());
#endif

			std::string auth_key(reinterpret_cast<char*>(&token.data()[32]), 24);
			std::string session_key("\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37\x13\x37", 24);

			// client_ticket
			auth_ticket_t ticket{};
			std::memset(&ticket, 0x0, sizeof ticket);
			ticket.m_magicNumber = 0x0EFBDADDE;
			ticket.m_type = 0;
			ticket.m_titleID = title_id;
			ticket.m_timeIssued = static_cast<uint32_t>(time(nullptr));
			ticket.m_timeExpires = ticket.m_timeIssued + 30000;
			ticket.m_licenseID = 0;
			ticket.m_userID = reinterpret_cast<uint64_t>(&token.data()[56]);
			strncpy_s(ticket.m_username, sizeof(ticket.m_username), reinterpret_cast<char*>(&token.data()[64]), 64);
			std::memcpy(ticket.m_sessionKey, session_key.data(), 24);

			const auto iv = utils::cryptography::tiger::compute(std::string(reinterpret_cast<char*>(&iv_seed), 4));
			std::string ticket_enc = utils::cryptography::des3::encrypt(
				std::string(reinterpret_cast<char*>(&ticket), sizeof(ticket)), iv, auth_key);
			std::string ticket_b64 = utils::cryptography::base64::encode((const unsigned char*)ticket_enc.c_str(), 128);

			// server_ticket
			uint8_t auth_data[128];
			std::memset(&auth_data, 0, sizeof auth_data);
			std::memcpy(auth_data, session_key.data(), 24);
			std::string auth_data_b64 = utils::cryptography::base64::encode(auth_data, 128);

			demonware::set_session_key(session_key);

			// header time
			char date[64];
			time_t now = time(0);
			tm gmtm;
			gmtime_s(&gmtm, &now);
			strftime(date, 64, "%a, %d %b %G %T", &gmtm);

			// json content
			std::string content;
			content.append("{\"auth_task\": \"29\",");
			content.append("\"code\": \"700\",");
			content.append(utils::string::va("\"iv_seed\": \"%u\",", iv_seed));
			content.append(utils::string::va("\"client_ticket\": \"%s\",", ticket_b64.c_str()));
			content.append(utils::string::va("\"server_ticket\": \"%s\",", auth_data_b64.c_str()));
			content.append("\"client_id\": \"\",");
			content.append("\"account_type\": \"steam\",");
			content.append("\"crossplay_enabled\": false,");
			content.append("\"loginqueue_eanbled\": false,");
			content.append("\"lsg_endpoint\": null,");
			content.append("}");

			// http stuff
			std::string result;
			result.append("HTTP/1.1 200 OK\r\n");
			result.append("Server: TornadoServer/4.5.3\r\n");
			result.append("Content-Type: application/json\r\n");
			result.append(utils::string::va("Date: %s GMT\r\n", date));
			result.append(utils::string::va("Content-Length: %d\r\n\r\n", content.size()));
			result.append(content);
			
			raw_reply reply(result);
			this->send_reply(&reply);

#ifdef DEBUG
			printf("[demonware]: [auth]: user successfully authenticated.\n");
#endif
		}
	}
}
