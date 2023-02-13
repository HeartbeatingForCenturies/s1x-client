#pragma once

namespace party
{
	void reset_connect_state();

	void connect(const game::netadr_s& target);
	void start_map(const std::string& mapname);

	void clear_sv_motd();
	int server_client_count();

	[[nodiscard]] int get_client_num_by_name(const std::string& name);

	[[nodiscard]] int get_client_count();
	[[nodiscard]] int get_bot_count();

	[[nodiscard]] game::netadr_s& get_target();
}
