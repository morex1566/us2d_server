#pragma once
#include "def.h"

/// <summary>
/// 전달 받은 패킷 처리하는 인터페이스 
/// </summary>
class ipacket_handler : public std::enable_shared_from_this<ipacket_handler>
{
public:
    virtual ~ipacket_handler() {}

    /// <summary>
    /// 수신한 패킷에 대한 이벤트
    /// </summary>
    /// <param name="serialized_payload">수신한 패킷 데이터</param>
    virtual void send(std::shared_ptr<google::protobuf::Message> serialized_payload) = 0;
};