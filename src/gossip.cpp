#include "gossip.h"

namespace gossip {
Peer::Peer(std::string peer_id, std::string peer_address)
    : _id(peer_id), _address(peer_address){};

}