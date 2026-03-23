import os
import re
import shutil

base_dir = r"c:/Users/morex/Documents/us2d_server/src"

# 1. Remove dispatcher from net.common
common_disp = os.path.join(base_dir, "net.common", "dispatcher.h")
if os.path.exists(common_disp):
    os.remove(common_disp)
    print("Removed net.common/dispatcher.h")

# 2. Create dispatcher in net.logic instead
dispatcher_h = """#pragma once
#include <functional>
#include <unordered_map>
#include <cstdint>
#include <iostream>

namespace net::logic
{
    using SessionId = uint64_t;
    using PacketHandler = std::function<void(SessionId, const uint8_t* payload, size_t size)>;

    class PacketDispatcher
    {
    private:
        std::unordered_map<int, PacketHandler> handlers_;
        PacketDispatcher() = default;

    public:
        static PacketDispatcher& get_instance() {
            static PacketDispatcher instance;
            return instance;
        }

        void Register(int packet_id, PacketHandler handler)
        {
            handlers_[packet_id] = std::move(handler);
        }

        void Dispatch(SessionId session_id, int packet_id, const uint8_t* payload, size_t size)
        {
            if (auto it = handlers_.find(packet_id); it != handlers_.end())
            {
                it->second(session_id, payload, size);
            }
            else
            {
                std::cout << "Unhandled packet ID: " << packet_id << std::endl;
            }
        }
    };
}
"""
with open(os.path.join(base_dir, "net.logic", "dispatcher.h"), "w", encoding="utf-8") as f:
    f.write(dispatcher_h)

# 3. Update net.logic/logic.h and cpp to use internal dispatcher
logic_h = """#pragma once
#include "dispatcher.h"

namespace net::logic
{
    void RegisterHandlers();
}
"""
with open(os.path.join(base_dir, "net.logic", "logic.h"), "w", encoding="utf-8") as f:
    f.write(logic_h)

logic_cpp = """#include "pch.h"
#include "logic.h"

namespace net::logic
{
    void HandleLoginReq(SessionId session_id, const uint8_t* payload, size_t size)
    {
        // Example implementation decoupled from net.core
    }

    void RegisterHandlers()
    {
        PacketDispatcher::get_instance().Register(1001, HandleLoginReq);
    }
}
"""
with open(os.path.join(base_dir, "net.logic", "logic.cpp"), "w", encoding="utf-8") as f:
    f.write(logic_cpp)
print("Moved dispatcher to net.logic and updated logic files.")

# 4. Modify net.core.vcxproj so net.core can depend on net.logic (to call logic::Dispatcher)
vcxproj_path = os.path.join(base_dir, "net.core", "net.core.vcxproj")
with open(vcxproj_path, "r", encoding="utf-8") as f:
    vcxproj = f.read()

if "net.logic.lib" not in vcxproj:
    vcxproj = vcxproj.replace("net.packet.lib;", "net.packet.lib;net.logic.lib;")
if "src\\net.logic" not in vcxproj:
    vcxproj = vcxproj.replace("src\\net.packet;", "src\\net.packet;$(SolutionDir)src\\net.logic;")

with open(vcxproj_path, "w", encoding="utf-8") as f:
    f.write(vcxproj)

print("Updated net.core.vcxproj to link and include net.logic")

# 5. Inject dispatch call into net.core/connection.cpp
connection_cpp_path = os.path.join(base_dir, "net.core", "connection.cpp")
with open(connection_cpp_path, "r", encoding="utf-8") as f:
    conn_cpp = f.read()

# Replace the empty request block with the dispatch call
target_code = """        const net::protocol::packet* full_packet = net::protocol::Getpacket(recv_buffer.data().data());
        {
            
        }"""
replacement_code = """        const net::protocol::packet* full_packet = net::protocol::Getpacket(recv_buffer.data().data());
        {
            // net::logic에서 관리하는 dispatcher를 통해 패킷 처리
            net::logic::PacketDispatcher::get_instance().Dispatch(
                session_id, 
                full_packet->header()->id(), 
                recv_buffer.data().data() + sizeof(net::protocol::packet_header), 
                payload_size
            );
        }"""
conn_cpp = conn_cpp.replace(target_code, replacement_code)
# Also add the include for it if not exists.
if '#include "../net.logic/dispatcher.h"' not in conn_cpp:
    conn_cpp = conn_cpp.replace('#include "system_config.h"', '#include "system_config.h"\n#include "../net.logic/dispatcher.h"\n#include "packet_generated.h"')

with open(connection_cpp_path, "w", encoding="utf-8") as f:
    f.write(conn_cpp)

print("Injected Dispatch logic into connection.cpp successfully!")
