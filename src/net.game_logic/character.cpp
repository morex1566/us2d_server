#include "pch.h"
#include "character.h"
#include "transformation.pb.h"

namespace net::game_logic
{
	void on_character_move(std::shared_ptr<google::protobuf::Message> payload)
	{
		auto transform_payload = std::static_pointer_cast<transformation>(payload);

		std::cout << "x : " << transform_payload->x() << std::endl;
		std::cout << "y : " << transform_payload->y() << std::endl;
	}
}