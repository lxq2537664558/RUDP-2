//
//  packet.h
//  RUDP
//
//  Created by Timothy Smale on 2016/03/24.
//  Copyright (c) 2016 Timothy Smale. All rights reserved.
//

#ifndef RUDP_packet_h
#define RUDP_packet_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <RUDP/util.h>
#include <RUDP/platform.h>

namespace RUDP
{
    typedef uint8_t ChannelId;
    typedef uint16_t PacketId;
    
    const size_t PacketSize = 512;
    const size_t MaxChannels = UINT8_MAX;
    
    enum PacketFlag : uint8_t
    {
        PacketFlag_None            = 0,
        PacketFlag_ConfirmDelivery = 1,
        PacketFlag_IsAck           = 1 << 2,
        PacketFlag_InOrder         = 1 << 3,
        PacketFlag_EndOfMessage    = 1 << 4,
        PacketFlag_StartOfMessage  = 1 << 5
    };
    
    inline const char *PacketFlag_ToString(PacketFlag flag)
    {
        switch(flag)
        {
                RUDP_STRINGIFY_CASE(PacketFlag_None);
                RUDP_STRINGIFY_CASE(PacketFlag_IsAck);
                RUDP_STRINGIFY_CASE(PacketFlag_ConfirmDelivery);
                RUDP_STRINGIFY_CASE(PacketFlag_EndOfMessage);
                RUDP_STRINGIFY_CASE(PacketFlag_StartOfMessage);
                RUDP_STRINGIFY_CASE(PacketFlag_InOrder);
        }
        
        return "UNKNOWN";
    }
    
    inline void operator |=(PacketFlag &a, const PacketFlag &b)
    {
        a = (PacketFlag)(((uint8_t)a) | ((uint8_t)b));
    }
    
    inline void operator &=(PacketFlag &a, const PacketFlag &b)
    {
        a = (PacketFlag)(((uint8_t)a) & ((uint8_t)b));
    }
    
    inline PacketFlag operator ~(const PacketFlag &a)
    {
        return (PacketFlag)(~((uint8_t)a));
    }
    
    RUDP_PACKEDSTRUCT(
                      struct PacketHeader
                      {
                          RUDP::PacketId m_packetId;
                          RUDP::ChannelId m_channelId;
                          RUDP::PacketFlag m_flags;
                      });
    
    class Packet
    {
    private:
        char m_buffer[RUDP::PacketSize];
        sockaddr_storage m_targetAddr;
        uint64_t m_timestamp;
        uint16_t m_readPosition;
        uint16_t m_writePosition;
        
    public:
        Packet() : m_readPosition(0), m_writePosition(0), m_timestamp(0)
        {
            memset(&m_targetAddr, 0, sizeof(m_targetAddr));
        }
        
        void setTimestamp(uint64_t ms);
        uint64_t getTimestamp();
        
        void setHeader(RUDP::PacketHeader *header);
        RUDP::PacketHeader *getHeader();
        
        void setTargetAddr(sockaddr_storage *addr);
        sockaddr_storage * getTargetAddr();
        
        uint16_t read(RUDP::Packet *buffer, size_t len);
        uint16_t write(RUDP::Packet *buffer, size_t len);
        
        template <typename T>
        inline uint16_t read(const T *buffer, size_t amount)
        {
            size_t numAvailable = RUDP::PacketSize - m_readPosition - sizeof(RUDP::PacketHeader);
            amount *= sizeof(T);
            
            if(amount > numAvailable)
            {
                amount = numAvailable;
            }
            
            memcpy((void*)buffer, m_buffer + m_readPosition + sizeof(RUDP::PacketHeader), amount);
            m_readPosition += amount;
            
            return amount / sizeof(T);
        }
        
        template <typename T>
        inline uint16_t write(const T *buffer, size_t amount)
        {
            size_t numAvailable = RUDP::PacketSize - m_writePosition - sizeof(RUDP::PacketHeader);
            amount *= sizeof(T);
            
            if(amount > numAvailable)
            {
                amount = numAvailable;
            }
            
            memcpy(m_buffer + m_writePosition + sizeof(RUDP::PacketHeader), buffer, amount);
            m_writePosition += amount;
            
            return amount / sizeof(T);
        }
        
        const char *getDataPtr();
        const char *getUserDataPtr();
        
        uint16_t getTotalSize();
        uint16_t getUserDataSize();
        
        void resetReadPosition();
        void resetWritePosition();
        uint16_t getReadPosition();
        uint16_t getWritePosition();
        bool setWritePosition(uint16_t len);
        bool setReadPosition(uint16_t len);
    };
    
    struct MessageStart
    {
        RUDP::Packet *m_first;
        RUDP::Packet *m_last;
        bool m_isAvailable;
        
        MessageStart() : m_first(NULL), m_last(NULL), m_isAvailable(false) {}
    };
}

#endif
