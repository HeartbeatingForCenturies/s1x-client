#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game_module.hpp"

#include <utils/hook.hpp>
#include <utils/thread.hpp>

#include "game/game.hpp"

#include "game/demonware/demonware.hpp"

#define TCP_BLOCKING true
#define UDP_BLOCKING false

namespace demonware
{
	namespace
	{
		void bd_logger_stub(const char* const function, const char* const msg, ...)
		{
			static auto* enabled =
				game::Dvar_RegisterBool("bd_logger_enabled", false, game::DVAR_FLAG_SAVED, "bdLogger");
			if (!enabled->current.enabled)
			{
				return;
			}

			char buffer[2048];

			va_list ap;
			va_start(ap, msg);

			vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, ap);
			printf("%s: %s\n", function, buffer);

			va_end(ap);
		}
	}

	volatile bool exit_server;
	std::thread server_thread;
	std::recursive_mutex server_mutex;
	std::map<SOCKET, bool> sockets_blocking;
	std::map<SOCKET, server_ptr> sockets;
	std::map<std::uint32_t, server_ptr> servers;

	void register_server(server_ptr server)
	{
		std::lock_guard<std::recursive_mutex> $(server_mutex);

		servers[server->address()] = server;
	}

	auto find_server_by_address(const std::uint32_t address) -> server_ptr
	{
		std::lock_guard<std::recursive_mutex> $(server_mutex);

		const auto it = servers.find(address);

		if (it != servers.end())
		{
			return it->second;
		}

		return server_ptr(nullptr);
	}

	auto find_server_by_name(const std::string& name) -> server_ptr
	{
		std::lock_guard<std::recursive_mutex> $(server_mutex);

		return find_server_by_address(utils::cryptography::jenkins_one_at_a_time::compute(name));
	}

	auto find_server_by_socket(const SOCKET socket) -> server_ptr
	{
		std::lock_guard<std::recursive_mutex> $(server_mutex);

		const auto it = sockets.find(socket);

		if (it != sockets.end())
		{
			return it->second;
		}

		return server_ptr(nullptr);
	}

	auto socket_link(const SOCKET socket, std::uint32_t address) -> bool
	{
		std::lock_guard<std::recursive_mutex> $(server_mutex);

		const auto server = find_server_by_address(address);

		if (!server) return false;

		sockets[socket] = server;

		return true;
	}

	void socket_unlink(const SOCKET socket)
	{
		std::lock_guard<std::recursive_mutex> $(server_mutex);

		const auto it = sockets.find(socket);

		if (it != sockets.end())
		{
			sockets.erase(it);
		}
	}

	auto socket_is_blocking(const SOCKET socket, const bool def) -> bool
	{
		std::lock_guard<std::recursive_mutex> $(server_mutex);

		if (sockets_blocking.find(socket) != sockets_blocking.end())
		{
			return sockets_blocking[socket];
		}

		return def;
	}

	void remove_blocking_socket(const SOCKET socket)
	{
		std::lock_guard<std::recursive_mutex> $(server_mutex);

		const auto it = sockets_blocking.find(socket);

		if (it != sockets_blocking.end())
		{
			sockets_blocking.erase(it);
		}
	}

	void add_blocking_socket(const SOCKET socket, const bool block)
	{
		std::lock_guard<std::recursive_mutex> $(server_mutex);

		sockets_blocking[socket] = block;
	}

	void server_main()
	{
		exit_server = false;

		while (!exit_server)
		{
			std::unique_lock<std::recursive_mutex> $(server_mutex);

			for (auto& server : servers)
			{
				server.second->frame();
			}

			$.unlock();

			std::this_thread::sleep_for(50ms);
		}
	}

	// WINSOCK

	namespace io
	{
		hostent* gethostbyname_stub(const char* name)
		{
#ifdef DEBUG
			printf("[ network ]: [gethostbyname]: \"%s\"\n", name);
#endif

			const auto server = find_server_by_name(name);

			if (server)
			{
				static thread_local in_addr address;
				address.s_addr = server->address();

				static thread_local in_addr* addr_list[2];
				addr_list[0] = &address;
				addr_list[1] = nullptr;

				static thread_local hostent host;
				host.h_name = const_cast<char*>(name);
				host.h_aliases = nullptr;
				host.h_addrtype = AF_INET;
				host.h_length = sizeof(in_addr);
				host.h_addr_list = reinterpret_cast<char**>(addr_list);

				return &host;
			}

#pragma warning(push)
#pragma warning(disable: 4996)
			return gethostbyname(name);
#pragma warning(pop)
		}

		int connect_stub(SOCKET s, const struct sockaddr* addr, const int len)
		{
			if (len == sizeof(sockaddr_in))
			{
				const auto* in_addr = reinterpret_cast<const sockaddr_in*>(addr);
				if (socket_link(s, in_addr->sin_addr.s_addr)) return 0;
			}

			return connect(s, addr, len);
		}

		int closesocket_stub(SOCKET s)
		{
			remove_blocking_socket(s);
			socket_unlink(s);

			return closesocket(s);
		}

		int send_stub(SOCKET s, const char* buf, int len, int flags)
		{
			auto server = find_server_by_socket(s);

			if (server)
			{
				return server->recv(buf, len);
			}

			return send(s, buf, len, flags);
		}

		int recv_stub(SOCKET s, char* buf, int len, int flags)
		{
			auto server = find_server_by_socket(s);

			if (server)
			{
				if (server->pending_data())
				{
					return server->send(buf, len);
				}
				else
				{
					WSASetLastError(WSAEWOULDBLOCK);
					return -1;
				}
			}

			return recv(s, buf, len, flags);
		}

		int sendto_stub(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen)
		{
			const auto* in_addr = reinterpret_cast<const sockaddr_in*>(to);
			auto server = find_server_by_address(in_addr->sin_addr.s_addr);

			if (server)
			{
				socket_link(s, in_addr->sin_addr.s_addr);
				return server->recv(buf, len);
			}

			return sendto(s, buf, len, flags, to, tolen);
		}

		int recvfrom_stub(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen)
		{
			auto server = find_server_by_socket(s);

			if (server)
			{
				while (socket_is_blocking(s, UDP_BLOCKING) && !server->pending_data())
				{
					std::this_thread::sleep_for(1ms);
				}

				if (server->pending_data())
				{
					return server->send(buf, len);
				}
				else
				{
					WSASetLastError(WSAEWOULDBLOCK);
					return -1;
				}
			}

			return recvfrom(s, buf, len, flags, from, fromlen);
		}

		int select_stub(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
		{
			int result = 0;
			std::vector<SOCKET> read_sockets;
			std::vector<SOCKET> write_sockets;

			for (auto& s : sockets)
			{
				if (readfds)
				{
					if (FD_ISSET(s.first, readfds))
					{
						if (s.second->pending_data())
						{
							read_sockets.push_back(s.first);
							FD_CLR(s.first, readfds);
						}
					}
				}

				if (writefds)
				{
					if (FD_ISSET(s.first, writefds))
					{
						write_sockets.push_back(s.first);
						FD_CLR(s.first, writefds);
					}
				}

				if (exceptfds)
				{
					if (FD_ISSET(s.first, exceptfds))
					{
						FD_CLR(s.first, exceptfds);
					}
				}
			}

			if ((!readfds || readfds->fd_count == 0) && (!writefds || writefds->fd_count == 0))
			{
				timeout->tv_sec = 0;
				timeout->tv_usec = 0;
			}

			result = select(nfds, readfds, writefds, exceptfds, timeout);
			if (result < 0) result = 0;

			for (size_t i = 0; i < read_sockets.size(); i++)
			{
				if (readfds)
				{
					FD_SET(read_sockets.at(i), readfds);
					result++;
				}
			}

			for (size_t i = 0; i < write_sockets.size(); i++)
			{
				if (writefds)
				{
					FD_SET(write_sockets.at(i), writefds);
					result++;
				}
			}

			return result;
		}

		int ioctlsocket_stub(const SOCKET s, const long cmd, u_long* argp)
		{
			if (static_cast<unsigned long>(cmd) == (FIONBIO))
			{
				add_blocking_socket(s, *argp == 0);
			}

			return ioctlsocket(s, cmd, argp);
		}
	}

	class component final : public component_interface
	{
	public:
		component()
		{
			/*register_server(std::make_shared<demonware::server_stun>("s1-stun.us.demonware.net"));
			register_server(std::make_shared<demonware::server_stun>("s1-stun.eu.demonware.net"));
			register_server(std::make_shared<demonware::server_stun>("s1-stun.jp.demonware.net"));
			register_server(std::make_shared<demonware::server_stun>("s1-stun.au.demonware.net"));

			register_server(std::make_shared<demonware::server_stun>("stun.us.demonware.net"));
			register_server(std::make_shared<demonware::server_stun>("stun.eu.demonware.net"));
			register_server(std::make_shared<demonware::server_stun>("stun.jp.demonware.net"));
			register_server(std::make_shared<demonware::server_stun>("stun.au.demonware.net"));*/

			register_server(std::make_shared<demonware::server_auth3>("aw-pc-auth3.prod.demonware.net"));
			register_server(std::make_shared<demonware::server_lobby>("aw-pc-lobby.prod.demonware.net"));
		}

		void post_load() override
		{
			server_thread = utils::thread::create_named_thread("Demonware", server_main);
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
			if (library == "WS2_32.dll")
			{
				if (function == "#3") return io::closesocket_stub;
				if (function == "#4") return io::connect_stub;
				if (function == "#10") return io::ioctlsocket_stub;
				if (function == "#16") return io::recv_stub;
				if (function == "#17") return io::recvfrom_stub;
				if (function == "#18") return io::select_stub;
				if (function == "#19") return io::send_stub;
				if (function == "#20") return io::sendto_stub;
				if (function == "#52") return io::gethostbyname_stub;
			}

			return nullptr;
		}

		void post_unpack() override
		{
			utils::hook::jump(SELECT_VALUE(0x140575880, 0x1406C0080), bd_logger_stub);

			if (game::environment::is_sp())
			{
				utils::hook::set<uint8_t>(0x1405632E0, 0xC3); // bdAuthSteam
				utils::hook::set<uint8_t>(0x1402DF2C0, 0xC3); // dwNet
				return;
			}

			utils::hook::set<uint8_t>(0x140698BB2, 0x0); // CURLOPT_SSL_VERIFYPEER
			utils::hook::set<uint8_t>(0x140698B69, 0xAF); // CURLOPT_SSL_VERIFYHOST
			utils::hook::set<uint8_t>(0x14088D0E8, 0x0); // HTTPS -> HTTP

			utils::hook::set<uint8_t>(0x140437CC0, 0xC3); // SV_SendMatchData
		}

		void pre_destroy() override
		{
			exit_server = true;
			if (server_thread.joinable())
			{
				server_thread.join();
			}

			servers.clear();
		}
	};
}

REGISTER_COMPONENT(demonware::component)
