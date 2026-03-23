import os
import re

base_dir = r"c:/Users/morex/Documents/us2d_server/src"

# 1. Remove dispatcher from net.logic
logic_disp = os.path.join(base_dir, "net.logic", "dispatcher.h")
if os.path.exists(logic_disp):
    os.remove(logic_disp)
    print("Removed net.logic/dispatcher.h")

# 2. Create dispatcher in net.packet instead
dispatcher_h = """#pragma once
#include <functional>
#include <unordered_map>
#include <cstdint>
#include <iostream>

namespace net::packet
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
with open(os.path.join(base_dir, "net.packet", "dispatcher.h"), "w", encoding="utf-8") as f:
    f.write(dispatcher_h)

print("Created src/net.packet/dispatcher.h (namespace net::packet)")

# 3. Revert net.core.vcxproj dependencies (remove net.logic)
vcxproj_path = os.path.join(base_dir, "net.core", "net.core.vcxproj")
with open(vcxproj_path, "r", encoding="utf-8") as f:
    vcxproj = f.read()

vcxproj = vcxproj.replace("net.packet.lib;net.logic.lib;", "net.packet.lib;")
vcxproj = vcxproj.replace("src\\net.packet;$(SolutionDir)src\\net.logic;", "src\\net.packet;")
with open(vcxproj_path, "w", encoding="utf-8") as f:
    f.write(vcxproj)

print("Reverted net.core.vcxproj to remove net.logic dependency.")

# 4. Modify logic files to use net.packet::PacketDispatcher
logic_h = """#pragma once
#include "../net.packet/dispatcher.h"

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
    void HandleLoginReq(net::packet::SessionId session_id, const uint8_t* payload, size_t size)
    {
        // Example implementation decoupled from net.core
    }

    void RegisterHandlers()
    {
        net::packet::PacketDispatcher::get_instance().Register(1001, HandleLoginReq);
    }
}
"""
with open(os.path.join(base_dir, "net.logic", "logic.cpp"), "w", encoding="utf-8") as f:
    f.write(logic_cpp)
print("Updated logic.h and logic.cpp to use net::packet::PacketDispatcher")

# 5. Modify connection.cpp to use net::packet::PacketDispatcher
connection_cpp_path = os.path.join(base_dir, "net.core", "connection.cpp")
with open(connection_cpp_path, "r", encoding="utf-8") as f:
    conn_cpp = f.read()

# Replace namespace references
conn_cpp = conn_cpp.replace('net::logic::PacketDispatcher::get_instance().Dispatch', 'net::packet::PacketDispatcher::get_instance().Dispatch')
conn_cpp = conn_cpp.replace('#include "../net.logic/dispatcher.h"', '#include "../net.packet/dispatcher.h"')

with open(connection_cpp_path, "w", encoding="utf-8") as f:
    f.write(conn_cpp)

print("Updated connection.cpp to use net::packet::PacketDispatcher")
