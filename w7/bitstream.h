#pragma once

#include <cstring>
#include <cassert>

static uint32_t uint8_max = std::numeric_limits<uint8_t>::max();
static uint32_t uint16_max = std::numeric_limits<uint16_t>::max();
static uint32_t uint32_max = std::numeric_limits<uint32_t>::max();

class WriteBitstream
{
public:
  WriteBitstream(ENetPacket* p) : packet(p), data_ptr(p->data), capacity(p->dataLength) {}
  template<typename T>
  void write(const T& data)
  {    
    memcpy(data_ptr + offset, reinterpret_cast<const uint8_t*>(&data), sizeof(T));
    offset += sizeof(T);
  }
  void writePackedUint32(uint32_t v)
  {
    if (v < uint8_max / 2)
    {
      uint8_t val = v; /// 0xxxxx
      memcpy(data_ptr + offset, reinterpret_cast<const uint8_t*>(&val), sizeof(uint8_t));
      offset += sizeof(uint8_t);
      return;
    }
    if (v < uint16_max / 4)
    {
		  uint16_t val = (2 << 14) + v; /// 10xxxx
      memcpy(data_ptr + offset, reinterpret_cast<const uint8_t*>(&val), sizeof(uint16_t));
      offset += sizeof(uint16_t);
      return;
    }
    if (v < uint32_max / 4)
    {
      int32_t val = (3 << 30) + v; /// 11xxxx
      memcpy(data_ptr + offset, reinterpret_cast<const uint8_t*>(&val), sizeof(int32_t));
      offset += sizeof(int32_t);
      return;
    }
    std::cout << "Can't pack value\n";
    assert(true);
  }
  void writePackedUint16(uint16_t v)
  {
    if (v < uint8_max / 2)
    {
      uint8_t val = v; /// 0xxxxx
      memcpy(data_ptr + offset, reinterpret_cast<const uint8_t*>(&val), sizeof(uint8_t));
      offset += sizeof(uint8_t);
      return;
    }
    if (v < uint16_max / 2)
    {
		  uint16_t val = (1 << 15) + v; /// 1xxxxx
      memcpy(data_ptr + offset, reinterpret_cast<const uint8_t*>(&val), sizeof(uint16_t));
      offset += sizeof(uint16_t);
      return;
    }
    std::cout << "Can't pack value\n";
    assert(true);
  }
  void compress()
  {
    if (offset < capacity)
    {
      enet_packet_resize(packet, offset);
    }
  }

private:
  ENetPacket* packet;
  uint8_t* data_ptr;
  uint32_t capacity = 0;
  uint32_t offset = 0;
};

class ReadBitstream
{
public:
  ReadBitstream(uint8_t* p) : ptr(p) {}

  template<typename T>
  void read(T& data) 
  {
    memcpy(reinterpret_cast<uint8_t*>(&data), ptr + offset, sizeof(T));
    offset += sizeof(T);
  }
  void readPackedUint32(uint32_t& v)
  {
    uint8_t val8;
    memcpy(reinterpret_cast<uint8_t*>(&val8), ptr + offset, sizeof(uint8_t));
    uint8_t val_for_check = val8; 
    if (val_for_check >> 7 == 0)
    {
      v = val8;
      offset += sizeof(uint8_t);
      return;
    }
    if (val_for_check >> 6 == 2)
    {
      uint16_t val16;
      memcpy(reinterpret_cast<uint8_t*>(&val16), ptr + offset, sizeof(uint16_t));
      v = val16 & 0x3fff;
      offset += sizeof(uint16_t);
      return;
    }
    if (val_for_check >> 6 == 3)
    {
      uint32_t val32;
      memcpy(reinterpret_cast<uint8_t*>(&val32), ptr + offset, sizeof(uint32_t));
      v = val32 & 0x3fffffff;
      offset += sizeof(uint32_t);
      return;
    }

    std::cout << "Can't unpack value\n";
    assert(true);
  }

  void readPackedUint16(uint16_t& v)
  {
    uint8_t val8;
    memcpy(reinterpret_cast<uint8_t*>(&val8), ptr + offset, sizeof(uint8_t));
    uint8_t val_for_check = val8; 
    if (val_for_check >> 7 == 0)
    {
      v = val8;
      offset += sizeof(uint8_t);
      return;
    }
    if (val_for_check >> 7 == 1)
    {
      uint16_t val16;
      memcpy(reinterpret_cast<uint8_t*>(&val16), ptr + offset, sizeof(uint16_t));
      v = val16 & 0x7fff;
      offset += sizeof(uint16_t);
      return;
    }

    std::cout << "Can't unpack value\n";
    assert(true);
  }

private:
  uint8_t* ptr;
  uint32_t offset = 0;
};