#pragma once
#include "def.h"
#include "google/protobuf/message.h"

namespace net::packet
{
	// 캐릭터 이동
	void on_character_move(std::shared_ptr<google::protobuf::Message> payload);
}
