import os
import re

def update_packet_h():
    filepath = r"c:\Users\morex\Documents\us2d_server\src\net.packet\packet.h"
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. Update packet_handler
    content = content.replace("using packet_handler;", "using packet_handler = std::function<void(std::shared_ptr<packet_request>)>;")

    # 2. Update dispatch signature and call
    old_dispatch = """        static void dispatch(unsigned short packet_id, std::shared_ptr<uint8_t> payload_owner)
        {
            auto& handlers = get_handlers();
            if (auto it = handlers.find(packet_id); it != handlers.end())
            {
                it->second(payload_owner);
            }
            else
            {
                std::cout << "Unhandled packet ID: " << packet_id << std::endl;
            }
        }"""
    
    new_dispatch = """        static void dispatch(std::shared_ptr<packet_request> req)
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
        }"""
    content = content.replace(old_dispatch, new_dispatch)

    # 3. Update register_typed_handler
    old_register = """    template<typename T>
    static void register_typed_handler(unsigned short packet_id, void(*handler_func)(T))
    {
        // T가 const T* 이든 const T& 이든, 순수 구조체 이름(payload_type)만 뽑아냅니다.
        using payload_type = std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

        packet_dispatcher::map(packet_id, [handler_func](const uint8_t* payload, size_t size)
        {
            const auto* data = flatbuffers::GetRoot<payload_type>(payload);
            if (data != nullptr)
            {
                // 로직 함수의 매개변수가 포인터(const T*) 타입인 경우
                if constexpr (std::is_pointer_v<T>)
                {
                    handler_func(data);
                }
                // 로직 함수의 매개변수가 참조(const T&) 또는 값 타입인 경우
                else
                {
                    handler_func(*data);
                }
            }
            else
            {
                std::cerr << "Failed to parse flatbuffers for packet_id: " << packet_id << std::endl;
            }
        });
    }"""

    new_register = """    template<typename PayloadType>
    static void register_typed_handler(unsigned short packet_id, void(*handler_func)(std::shared_ptr<packet_request>, const PayloadType*))
    {
        packet_dispatcher::map(packet_id, [handler_func](std::shared_ptr<packet_request> req)
        {
            const uint8_t* payload_start = req->payload_owner.get() + sizeof(net::protocol::packet_header);
            const auto* data = flatbuffers::GetRoot<PayloadType>(payload_start);
            if (data != nullptr)
            {
                handler_func(req, data);
            }
            else
            {
                std::cerr << "Failed to parse flatbuffers for packet_id: " << req->packet_id << std::endl;
            }
        });
    }"""
    content = content.replace(old_register, new_register)

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    print("Updated packet.h")


def update_logic_h():
    filepath = r"c:\Users\morex\Documents\us2d_server\src\net.logic\logic.h"
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    old_handler = "static void on_transform(const net::protocol::transform* transform_in)"
    new_handler = "static void on_transform(std::shared_ptr<net::packet::packet_request> req, const net::protocol::transform* transform_in)"
    content = content.replace(old_handler, new_handler)

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    print("Updated logic.h")

if __name__ == "__main__":
    update_packet_h()
    update_logic_h()
