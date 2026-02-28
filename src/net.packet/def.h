#pragma once
#include <memory>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <concepts>



namespace google::protobuf { class Message; }

namespace net::packet { class iserver; }

namespace net::packet
{
	using payload = google::protobuf::Message;

	using payload_sptr = std::shared_ptr<payload>;

	using payload_handle = void(*)(payload_sptr, iserver*);

	using serialization_handle = payload_sptr(*)(const std::vector<uint8_t>&, uint32_t);
}