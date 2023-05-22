#pragma once
#include "bitstream.h"
#include "mathUtils.h"
#include <limits>
#include <cassert>
struct float2
{
  float data[2];
  float& operator [](size_t idx) { return data[idx]; }
};
struct float3
{
  float data[3];
  float& operator [](size_t idx) { return data[idx]; }
};
struct Interval
{
  float lo;
  float hi;
};
bool quantized_equal(float x, float y, Interval interval, int num_bits)
{
  int range = (1 << num_bits) - 1;
  float step = (interval.hi - interval.lo) / range;
  return abs(x - y) < step;
}
template<typename T>
T pack_float(float v, Interval interval, int num_bits)
{
  T range = (1 << num_bits) - 1;//std::numeric_limits<T>::max();
  return (T)(range * ((clamp(v, interval.lo, interval.hi) - interval.lo) / (interval.hi - interval.lo)));
}

template<typename T>
float unpack_float(T c, Interval interval, int num_bits)
{
  T range = (1 << num_bits) - 1;//std::numeric_limits<T>::max();
  return float(c) / range * (interval.hi - interval.lo) + interval.lo;
}
template<typename T, int num_bits>
struct PackedFloat
{
  T packedVal;
  PackedFloat(float v, Interval interval) { pack(v, interval); }
  PackedFloat(T compressed_val) : packedVal(compressed_val) {}
  void pack(float v, Interval interval) { packedVal = pack_float<T>(v, interval, num_bits); }
  float unpack(Interval interval) { return unpack_float<T>(packedVal, interval, num_bits); }
};
typedef PackedFloat<uint8_t, 4> float4bitsQuantized;