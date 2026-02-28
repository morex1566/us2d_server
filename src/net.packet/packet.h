#pragma once
#include "def.h"

namespace net::packet
{
    template <typename t_packet>
    concept is_protobuf_message = std::derived_from<t_packet, payload>;

    // 패킷 식별 ID
    enum class packet_id : uint8_t
    {
        none,
        transformation
    };

    // socket async_read() 후, 받은 데이터 스트림 -> google::protobuf 변환
    // t : protoc.exe로 생성한 패킷 클래스
    template <typename t>
    requires is_protobuf_message<t>
    payload_sptr deserialize_payload(const std::vector<uint8_t>& stream, uint32_t size)
    {
        // 탬플릿이 google::protobuf 가 아님
        static_assert
        (
            std::is_base_of_v<payload, t>, "t must derive from google::protobuf::Message"
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

    template <typename t>
    requires is_protobuf_message<t>
    bool serialize_payload(const t& t_payload, std::vector<uint8_t>& t_out_buffer)
    {
        // 1. 캐시된 크기 사용 (ByteSizeLong은 내부적으로 캐시를 업데이트함)
        const size_t payload_size = t_payload.ByteSizeLong();

        // 2. 출력 버퍼 크기 조정 (기존 메모리 재사용 유도)
        // resize 대신 기존 용량이 충분하면 할당을 피함
        t_out_buffer.clear();
        t_out_buffer.resize(payload_size);

        // 3. SerializeToArray는 내부적으로 빠르지만, 
        // 더 극단적인 성능이 필요하면 로우 레벨 포인터를 직접 제어
        if (!t_payload.SerializeToArray(t_out_buffer.data(), static_cast<int>(payload_size)))
        {
            t_out_buffer.clear();
            return false;
        }

        return true;
    }

    // [enum packet.id] 접근, 데이터 스트림 -> google::protobuf 변환 함수 호출
    inline static std::unordered_map<packet_id, serialization_handle> packet_serializer_map
    {
        // std::pair를 생략하고 중괄호로 묶습니다.
        { packet_id::transformation, &net::packet::deserialize_payload<transformation> }
    };


#pragma pack(push, 1)
    struct packet_header
    {
    public:
        // 패킷 식별 ID
        packet_id id = packet_id::none;

        // payload 사이즈
        uint32_t payload_size = 0;
    };
#pragma pack(pop)


    struct packet
    {
    public:
        packet_header header;

        // google protobuf로 만든 class를 SerializeToArray()
        std::vector<uint8_t> payload;
    };

    /// <summary>
    /// 패킷을 서버에서 처리하기 위해 사용하는 명령
    /// </summary>
    struct packet_request
    {
    public:
        packet_request() = default;

        packet_request(packet_id id, payload_sptr payload, payload_handle handler)
            : id(id), payload(payload), handler(handler) { }

        packet_id id;

        payload_sptr payload;

        payload_handle handler;

        void execute()
        {
            if (handler && payload)
            {
                handler(payload);
            }
        }
    };
}