#pragma once
#include <functional>
#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <memory>
#include <type_traits>
#include "flatbuffers/flatbuffers.h"
#include "system_config.h"
#include "packet_generated.h"
#include "logger.h"

namespace net::packet
{
    struct packet_request;
    using packet_handler = std::function<void(std::shared_ptr<packet_request>)>;

    struct packet_request
    {
        uint16_t packet_id;
        std::shared_ptr<uint8_t> payload_owner;
        uint16_t payload_size;
    };

    class packet_dispatcher
    {
    private:
        // C++ Static Initialization Order Fiasco 방지를 위한 싱글톤 방식 유지
        static std::unordered_map<unsigned short, packet_handler>& get_handlers()
        {
            static std::unordered_map<unsigned short, packet_handler> handlers;
            return handlers;
        }

    public:
        static void map(unsigned short packet_id, packet_handler handler)
        {
            auto& handlers = get_handlers();
            handlers[packet_id] = std::move(handler);
        }

        static void dispatch(std::shared_ptr<packet_request> req)
        {
            auto& handlers = get_handlers();
            if (auto it = handlers.find(req->packet_id); it != handlers.end())
            {
                it->second(req);
            }
            else
            {
                std::cout << "Unhandled packet ID: " << req->packet_id << std::endl;
            }
        }
    };

    // --- 타입 추론을 위한 헬퍼 함수 ---
    // 핸들러 함수의 매개변수 타입(T)을 자동으로 추론합니다.
    template<typename packet_type>
    static void register_typed_handler(unsigned short packet_id, void(*handler_func)(std::shared_ptr<packet_request>, const packet_type*))
    {
        packet_dispatcher::map(packet_id, [handler_func](std::shared_ptr<packet_request> request)
        {
            const uint8_t* payload_start = request->payload_owner.get() + sizeof(net::protocol::packet_header);
            const auto* data = flatbuffers::GetRoot<packet_type>(payload_start);
            CHECK_RETURN_VOID(data == nullptr, "Failed to parse flatbuffers for packet_id: {}", request->packet_id);
            handler_func(request, data);
        });
    }
}

// 매크로 전개 순서를 제어하는 CONCAT 구현을 사용하여 __LINE__을 고유 이름으로 변환
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define REGISTER_HANDLER_IMPL(packet_id, handler_func, unique_id) \
static struct CONCAT(auto_registrar_, unique_id) \
{ \
    CONCAT(auto_registrar_, unique_id)() \
    { \
        net::packet::register_typed_handler(packet_id, handler_func); \
    } \
} CONCAT(_auto_registrar_instance_, unique_id);

#define REGISTER_HANDLER(packet_id, handler_func) \
    REGISTER_HANDLER_IMPL(packet_id, handler_func, __LINE__)
