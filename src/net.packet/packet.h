#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <concepts>
#include <iostream>
#pragma region include_"google/protoc.generated.h"
#include "transformation.pb.h"
#include "server_stats.pb.h"
#include "chat.pb.h"
// 여기에 계속해서 protoc파일 헤더 추가 <-----------------
#pragma endregion

namespace net::packet
{
    template <typename t_packet>
    concept is_protobuf_message = std::derived_from<t_packet, google::protobuf::Message>;

    // 패킷 식별 타입
    enum class packet_type : uint8_t
    {
        none,

        // system
        connected,
        disconnected,

        // game
        transformation,
        chat,

        // admin
        load_stats
    };

    // socket async_read() 후, 받은 데이터 스트림 -> google::protobuf 변환
    // t : protoc.exe로 생성한 패킷 클래스
    template <typename t>
    requires is_protobuf_message<t>
    std::shared_ptr<google::protobuf::Message> deserialize_payload(const std::vector<uint8_t>& stream, uint32_t size)
    {
        static_assert
        (
            std::is_base_of_v<google::protobuf::Message, t>, "t must derive from google::protobuf::Message"
        );

        auto msg = std::make_shared<t>();

        // 바이트 스트림 -> 클래스
        if (!msg->ParseFromArray(stream.data(), size))
        {
            std::cout << "data read failed" << std::endl;

            return nullptr;
        }

        return msg;
    }

    // raw buffer 기반 오버로드 (ts_memory_pool 슬롯 직접 사용)
    template <typename t>
    requires is_protobuf_message<t>
    std::shared_ptr<google::protobuf::Message> deserialize_payload(const uint8_t* data, uint32_t size)
    {
        static_assert
        (
            std::is_base_of_v<google::protobuf::Message, t>, "t must derive from google::protobuf::Message"
        );

        auto msg = std::make_shared<t>();

        if (!msg->ParseFromArray(data, size))
        {
            std::cout << "data read failed" << std::endl;

            return nullptr;
        }

        return msg;
    }

    // [enum packet.id] 접근, 데이터 스트림 -> google::protobuf 변환 함수 호출 (vector 기반)
    inline static std::unordered_map<packet_type, std::shared_ptr<google::protobuf::Message>(*)(const std::vector<uint8_t>&, uint32_t)> packet_deserializer_map
    {
        { packet_type::transformation, &net::packet::deserialize_payload<transformation> },
        { packet_type::chat, &net::packet::deserialize_payload<net::protocol::chat> },
        { packet_type::load_stats, &net::packet::deserialize_payload<server_stats> }
    };

    // raw buffer 기반 deserializer 맵 (ts_memory_pool 슬롯 직접 사용)
    inline static std::unordered_map<packet_type, std::shared_ptr<google::protobuf::Message>(*)(const uint8_t*, uint32_t)> packet_raw_deserializer_map
    {
        { packet_type::transformation, &net::packet::deserialize_payload<transformation> },
        { packet_type::chat, &net::packet::deserialize_payload<net::protocol::chat> },
        { packet_type::load_stats, &net::packet::deserialize_payload<server_stats> }
    };

#pragma pack(push, 1)
    struct packet_header
    {
    public:
        /// <summary>
        /// 패킷 구분용 ex. 공격, 이동, 채팅
        /// </summary>
        packet_type type = packet_type::none;

        /// <summary>
        /// 서버 접근용 토큰
        /// </summary>
        uint32_t connection_id = 0;

        /// <summary>
        /// 패킷 로스 측정용
        /// </summary>
        uint32_t seq_num = 0;

        /// <summary>
        /// 핑 측정용
        /// </summary>
        uint32_t timestamp = 0;

        /// <summary>
        /// payload 로드용
        /// </summary>
        uint32_t payload_size = 0;

        /// <summary>
        /// 데이터 변조 체크용
        /// </summary>
        uint8_t check_sum = 0;
    };
#pragma pack(pop)

    // 슬랩 슬롯 정렬 맞춤 (캐시 라인 경계, 64바이트)
    struct alignas(64) packet
    {
    public:

        packet_header header;

        std::vector<uint8_t> payload;
    };

    /// <summary>
    /// 서버의 recv 버퍼에 큐를 거는 용도
    /// </summary>
    struct packet_request
    {
    public:
        packet_request() = default;

        packet_request(packet_type type, uint64_t id, std::shared_ptr<google::protobuf::Message> payload)
            : type(type), id(id), payload(payload) { }

        packet_type type;

        uint64_t id;

        std::shared_ptr<google::protobuf::Message> payload;
    };

    /// <summary>
    /// 클라-서버 통신 데이터 (전송용 바이트 스트림 생성 유틸리티)
    /// </summary>
    inline std::vector<uint8_t> serialize(packet_type type, const google::protobuf::Message& payload)
    {
        size_t payload_size = payload.ByteSizeLong();
        size_t header_size = sizeof(packet_header);

        std::vector<uint8_t> buffer(header_size + payload_size);

        // 헤더 작성
        packet_header* header = reinterpret_cast<packet_header*>(buffer.data());
        header->type = type;
        header->payload_size = static_cast<uint32_t>(payload_size);

        // 페이로드 직렬화
        if (payload_size > 0)
        {
            payload.SerializeToArray(buffer.data() + header_size, static_cast<int>(payload_size));
        }

        return buffer;
    }

    /// <summary>
    /// 수신된 바이트 스트림을 해석하여 packet_request로 변환
    /// </summary>
    inline packet_request deserialize(packet_type type, uint64_t session_id, const std::vector<uint8_t>& stream, uint32_t size)
    {
        auto it = packet_deserializer_map.find(type);
        if (it != packet_deserializer_map.end())
        {
            auto decoded_payload = it->second(stream, size);
            return packet_request(type, session_id, decoded_payload);
        }

        return packet_request(packet_type::none, session_id, nullptr);
    }

    /// <summary>
    /// raw buffer 기반 deserialization (ts_memory_pool 슬롯 직접 사용)
    /// </summary>
    inline packet_request deserialize(packet_type type, uint64_t session_id, const uint8_t* data, uint32_t size)
    {
        auto it = packet_raw_deserializer_map.find(type);
        if (it != packet_raw_deserializer_map.end())
        {
            auto decoded_payload = it->second(data, size);
            return packet_request(type, session_id, decoded_payload);
        }

        return packet_request(packet_type::none, session_id, nullptr);
    }
}