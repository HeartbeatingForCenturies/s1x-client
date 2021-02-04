#pragma once

#include <std_include.hpp>

#include "bit_buffer.hpp"
#include "byte_buffer.hpp"
#include "data_types.hpp"
#include "reply.hpp"
#include "server.hpp"
#include "service.hpp"

//#include "services/bdTeams.hpp"           //  3
#include "services/bdStats.hpp"             //  4
//#include "services/bdMessaging.hpp"       //  6
#include "services/bdProfiles.hpp"          //  8
#include "services/bdStorage.hpp"           // 10
#include "services/bdTitleUtilities.hpp"    // 12
#include "services/bdBandwidthTest.hpp"     // 18
//#include "services/bdMatchMaking.hpp"     // 21
#include "services/bdCounters.hpp"          // 23
#include "services/bdDML.hpp"               // 27
#include "services/bdGroups.hpp"            // 28
//#include "services/bdCMail.hpp"           // 29
#include "services/bdFacebook.hpp"          // 36
#include "services/bdAnticheat.hpp"         // 38
#include "services/bdContentStreaming.hpp"  // 50
//#include "services/bdTags.hpp"            // 52
#include "services/bdUNK63.hpp"             // 63
#include "services/bdEventLog.hpp"          // 67
#include "services/bdRichPresence.hpp"      // 68
//#include "services/bdTitleUtilities2.hpp" // 72
#include "services/bdUNK80.hpp"
// AccountLinking                           // 86
#include "services/bdPresence.hpp"          //103
#include "services/bdUNK104.hpp"            //104 Marketing
#include "services/bdMatchMaking2.hpp"      //138
#include "services/bdMarketing.hpp"         //139

// servers
#include "servers/server_auth3.hpp"
#include "servers/server_lobby.hpp"
#include "servers/server_stun.hpp"

namespace demonware
{
	void derive_keys_s1();
	void queue_packet_to_hash(const std::string& packet);
	void set_session_key(const std::string& key);
	std::string get_decrypt_key();
	std::string get_encrypt_key();
	std::string get_hmac_key();
	std::string get_response_id();
}
