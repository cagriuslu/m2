#include <m2/multi_player/lockstep/ConnectionToClient.h>

using namespace m2;
using namespace m2::multiplayer;
using namespace m2::multiplayer::lockstep;

ConnectionToClient::ConnectionToClient(network::IpAddressAndPort address, MessagePasser& messagePasser)
	: _addressAndPort(std::move(address)), _messagePasser(messagePasser) {}
