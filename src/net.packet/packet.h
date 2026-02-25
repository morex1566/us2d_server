#pragma once
#include "def.h"

namespace net::packet
{
    // 패킷 식별 ID
    enum class packet_id : uint8_t
    {
        none,
        transformation
    };

    // 패킷 payload 읽기 위한 필수 정보

    #pragma pack(push, 1)
    class packet_header
    {
    public:
        // 패킷 식별 ID
        packet_id m_id = packet_id::none;

        // payload 사이즈
        uint32_t m_size = 0;
    };
    #pragma pack(pop)

    // packet_id + packet_payload + packet_handler 전부 포함
    class packet_request
    {
    public:
        packet_request() = default;

        packet_request
        (
            packet_id id,
            std::shared_ptr<google::protobuf::Message> payload,
            std::function<void(std::shared_ptr<google::protobuf::Message>)> handler
        ) 
            : m_id(id), m_payload(payload), m_handler(handler) { }

        packet_id m_id;

        std::function<void(std::shared_ptr<google::protobuf::Message>)> m_handler;

        std::shared_ptr<google::protobuf::Message> m_payload;
    };

    // socket async_read() 후, 받은 데이터 스트림 -> google::protobuf 변환
    // t : protoc.exe로 생성한 패킷 클래스
    template <typename t>
    std::shared_ptr<google::protobuf::Message> create_protoc_packet(const std::vector<uint8_t>& stream, uint32_t size)
    {
        // 탬플릿이 google::protobuf 가 아님
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

    // [enum packet.id] 접근, 데이터 스트림 -> google::protobuf 변환 함수 호출
    inline static std::unordered_map<
        packet_id,
        std::function<std::shared_ptr<google::protobuf::Message>(const std::vector<uint8_t>&, uint32_t)>> packet_creator_map
    {
        // std::pair를 생략하고 중괄호로 묶습니다.
        { packet_id::transformation, &net::packet::create_protoc_packet<transformation> }
    };
}