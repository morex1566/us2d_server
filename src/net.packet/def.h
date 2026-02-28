#pragma once
#include <memory>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <concepts>
#pragma region include_"google/protoc.generated.h"
#include "transformation.pb.h"
#include "server_stats.pb.h"
#include "chat.pb.h"
// 여기에 계속해서 protoc파일 헤더 추가 <-----------------
#pragma endregion


namespace google::protobuf { class Message; }

namespace net::packet
{
	using payload = google::protobuf::Message;

	using payload_sptr = std::shared_ptr<payload>;

	using payload_handle = std::function<void(payload_sptr)>;

	using serialization_handle = std::function<payload_sptr(const std::vector<uint8_t>&, uint32_t)>;
}