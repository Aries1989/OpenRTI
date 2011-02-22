/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include "RTI/HLAfloat64Interval.h"

#include <cmath>
#include <limits>
#include <sstream>
#include "VariableLengthDataImplementation.h"
#include "ValueImplementation.h"
#include "RTI/HLAfloat64Time.h"

DECLARE_VALUE_IMPLEMENTATION(HLAfloat64IntervalImpl, double)

static const HLAfloat64Time& toHLAfloat64Time(const rti1516::LogicalTime& logicalTime)
{
  const HLAfloat64Time* float64Time = dynamic_cast<const HLAfloat64Time*>(&logicalTime);
  if (!float64Time)
    throw rti1516::InvalidLogicalTime(logicalTime.implementationName());
  return *float64Time;
}

static const HLAfloat64Interval& toHLAfloat64Interval(const rti1516::LogicalTimeInterval& logicalTimeInterval)
{
  const HLAfloat64Interval* float64Interval = dynamic_cast<const HLAfloat64Interval*>(&logicalTimeInterval);
  if (!float64Interval)
    throw rti1516::InvalidLogicalTimeInterval(logicalTimeInterval.implementationName());
  return *float64Interval;
}

static inline bool isNaN(const double& fedTime)
{
#if 1
  // Code that here ourselfs, compilers might decide to optimize isnan away
  // because of some assumtions based on optimization flags

  // Neat trick:
  // A nan has all bits in its exponent set and any bit in the mantissa.
  // So a nan's bitpattern int representation can be written as
  //  0x7f800000 + x
  // where x > 0.
  // That means compute:
  //   t = 0x7f800000 - (0x7f800000 + x) = -x
  // and look for the sign of t.
  // If the exponent is not 0x7f800000 (that is less than 0x7f800000), the
  // resulting t value will be >= 0.
  
  union {
    double d;
    uint64_t u;
  } u;
  u.d = fedTime;

  // remove the sign bit since in the above the sign bit is not set
  int64_t i = (u.u & 0x7fffffffffffffffll);
  // compute the above t
  int64_t t = 0x7ff0000000000000ll - i;
  return t < 0;

#elif defined _WIN32
  return 0 != _isnan(fedTime);
#elif defined(__sun) || defined(__hpux)
  return isnan(fedTime);
#else
  return std::isnan(fedTime);
#endif
}

static inline double nextAfter(const double& fedTime, const double& direction)
{
#ifdef _WIN32
  return _nextafter(fedTime, direction);
#else
  return nextafter(fedTime, direction);
#endif
}

static inline bool signbitSet(const double& fedTime)
{
#ifdef HAVE_SIGNBIT
  return signbit(fedTime);
#else
  union {
    uint64_t i;
    double d;
  } u;
  u.d = fedTime;
  return 0 != (u.i & (uint64_t(1) << 63));
#endif
}

// We treat -0 as epsilon, so if we set from a double value we need to ensure that the sign bit is not set
// Note that this comparison also preserves nan's
static inline double clearSign(const double& fedTime)
{
  if (fedTime == 0 && signbitSet(fedTime))
    return 0;
  return fedTime;
}

HLAfloat64Interval::HLAfloat64Interval() :
  _impl(0)
{
}

HLAfloat64Interval::HLAfloat64Interval(double time) :
  _impl(0)
{
  HLAfloat64IntervalImpl::setValue(_impl, clearSign(time));
}

HLAfloat64Interval::HLAfloat64Interval(const rti1516::LogicalTimeInterval& logicalTimeInterval) :
  _impl(0)
{
  HLAfloat64IntervalImpl::assign(_impl, toHLAfloat64Interval(logicalTimeInterval)._impl);
}

HLAfloat64Interval::HLAfloat64Interval(const HLAfloat64Interval& float64Interval) :
  _impl(0)
{
  HLAfloat64IntervalImpl::assign(_impl, float64Interval._impl);
}

HLAfloat64Interval::~HLAfloat64Interval() throw ()
{
  HLAfloat64IntervalImpl::putAndDelete(_impl);
}

void
HLAfloat64Interval::setZero()
{
  HLAfloat64IntervalImpl::setValue(_impl, 0);
}

bool
HLAfloat64Interval::isZero() const
{
  double value = HLAfloat64IntervalImpl::getValue(_impl);
  if (isNaN(value))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  return 0 == value && !signbitSet(value);
}

void
HLAfloat64Interval::setEpsilon()
{
  HLAfloat64IntervalImpl::setValue(_impl, -0);
}

bool
HLAfloat64Interval::isEpsilon() const
{
  double value = HLAfloat64IntervalImpl::getValue(_impl);
  if (isNaN(value))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  return value == 0 && signbitSet(value);
}

HLAfloat64Interval&
HLAfloat64Interval::operator=(const rti1516::LogicalTimeInterval& logicalTimeInterval)
  throw (rti1516::InvalidLogicalTimeInterval)
{
  HLAfloat64IntervalImpl::assign(_impl, toHLAfloat64Interval(logicalTimeInterval)._impl);
  return *this;
}

void
HLAfloat64Interval::setToDifference(const rti1516::LogicalTime& minuend, const rti1516::LogicalTime& subtrahend)
  throw (rti1516::InvalidLogicalTime)
{
  HLAfloat64IntervalImpl::setValue(_impl, clearSign(toHLAfloat64Time(minuend).getTime() - toHLAfloat64Time(subtrahend).getTime()));
}

HLAfloat64Interval&
HLAfloat64Interval::operator+=(const rti1516::LogicalTimeInterval& logicalTimeInterval)
  throw (rti1516::InvalidLogicalTimeInterval)
{
  if (isEpsilon())
    throw rti1516::InvalidLogicalTimeInterval(L"Can not change an epsilon LogicalTimeInterval!");
  if (logicalTimeInterval.isEpsilon()) {
    double value = nextAfter(HLAfloat64IntervalImpl::getValue(_impl), std::numeric_limits<double>::infinity());
    HLAfloat64IntervalImpl::setValue(_impl, clearSign(value));
  } else {
    double value = HLAfloat64IntervalImpl::getValue(_impl) + HLAfloat64IntervalImpl::getValue(toHLAfloat64Interval(logicalTimeInterval)._impl);
    HLAfloat64IntervalImpl::setValue(_impl, clearSign(value));
  }
  return *this;
}

HLAfloat64Interval&
HLAfloat64Interval::operator-=(const rti1516::LogicalTimeInterval& logicalTimeInterval)
  throw (rti1516::InvalidLogicalTimeInterval)
{
  if (isEpsilon())
    throw rti1516::InvalidLogicalTimeInterval(L"Can not change an epsilon LogicalTimeInterval!");
  if (logicalTimeInterval.isEpsilon()) {
    double value = nextAfter(HLAfloat64IntervalImpl::getValue(_impl), -std::numeric_limits<double>::infinity());
    HLAfloat64IntervalImpl::setValue(_impl, clearSign(value));
  } else {
    double value = HLAfloat64IntervalImpl::getValue(_impl) - HLAfloat64IntervalImpl::getValue(toHLAfloat64Interval(logicalTimeInterval)._impl);
    HLAfloat64IntervalImpl::setValue(_impl, clearSign(value));
  }
  return *this;
}

bool
HLAfloat64Interval::operator>(const rti1516::LogicalTimeInterval& logicalTimeInterval) const
  throw (rti1516::InvalidLogicalTimeInterval)
{
  double left = HLAfloat64IntervalImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  double right = HLAfloat64IntervalImpl::getValue(toHLAfloat64Interval(logicalTimeInterval)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  return left > right;
}

bool
HLAfloat64Interval::operator<(const rti1516::LogicalTimeInterval& logicalTimeInterval) const
  throw (rti1516::InvalidLogicalTimeInterval)
{
  double left = HLAfloat64IntervalImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  double right = HLAfloat64IntervalImpl::getValue(toHLAfloat64Interval(logicalTimeInterval)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  return left < right;
}

bool
HLAfloat64Interval::operator==(const rti1516::LogicalTimeInterval& logicalTimeInterval) const
  throw (rti1516::InvalidLogicalTimeInterval)
{
  double left = HLAfloat64IntervalImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  double right = HLAfloat64IntervalImpl::getValue(toHLAfloat64Interval(logicalTimeInterval)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  return left == right;
}

bool
HLAfloat64Interval::operator>=(const rti1516::LogicalTimeInterval& logicalTimeInterval) const
  throw (rti1516::InvalidLogicalTimeInterval)
{
  double left = HLAfloat64IntervalImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  double right = HLAfloat64IntervalImpl::getValue(toHLAfloat64Interval(logicalTimeInterval)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  return left >= right;
}

bool
HLAfloat64Interval::operator<=(const rti1516::LogicalTimeInterval& logicalTimeInterval) const
  throw (rti1516::InvalidLogicalTimeInterval)
{
  double left = HLAfloat64IntervalImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  double right = HLAfloat64IntervalImpl::getValue(toHLAfloat64Interval(logicalTimeInterval)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTimeInterval(L"Can not comare with NaN!");
  return left <= right;
}

rti1516::VariableLengthData
HLAfloat64Interval::encode() const
{
  OpenRTI::VariableLengthData variableLengthData(8);
  variableLengthData.setFloat64BE(HLAfloat64IntervalImpl::getValue(_impl), 0);
  return rti1516::VariableLengthDataFriend::create(variableLengthData);
}

unsigned long
HLAfloat64Interval::encodedLength() const
{
  return 8;
}

unsigned long
HLAfloat64Interval::encode(void* buffer, unsigned long bufferSize) const
  throw (rti1516::CouldNotEncode)
{
  if (bufferSize < 8)
    throw rti1516::CouldNotEncode(L"Buffer size too short!");
  union {
    double d;
    uint64_t u;
  } u;
  u.d = HLAfloat64IntervalImpl::getValue(_impl);
  uint8_t* data = static_cast<uint8_t*>(buffer);
  data[0] = uint8_t(u.u >> 56);
  data[1] = uint8_t(u.u >> 48);
  data[2] = uint8_t(u.u >> 40);
  data[3] = uint8_t(u.u >> 32);
  data[4] = uint8_t(u.u >> 24);
  data[5] = uint8_t(u.u >> 16);
  data[6] = uint8_t(u.u >> 8);
  data[7] = uint8_t(u.u);
  return 8;
}

void
HLAfloat64Interval::decode(const rti1516::VariableLengthData& variableLengthData)
  throw (rti1516::InternalError, rti1516::CouldNotDecode)
{
  OpenRTI::VariableLengthData data = rti1516::VariableLengthDataFriend::readPointer(variableLengthData);
  if (data.size() < 8)
    throw rti1516::CouldNotDecode(L"Buffer size too short!");
  HLAfloat64IntervalImpl::setValue(_impl, data.getFloat64BE(0));
}

void
HLAfloat64Interval::decode(void* buffer, unsigned long bufferSize)
  throw (rti1516::InternalError, rti1516::CouldNotDecode)
{
  if (bufferSize < 8)
    throw rti1516::CouldNotDecode(L"Buffer size too short!");
  union {
    double d;
    uint64_t u;
  } u;
  const uint8_t* data = static_cast<const uint8_t*>(buffer);
  u.u = uint64_t(data[0]) << 56;
  u.u |= uint64_t(data[1]) << 48;
  u.u |= uint64_t(data[2]) << 40;
  u.u |= uint64_t(data[3]) << 32;
  u.u |= uint64_t(data[4]) << 24;
  u.u |= uint64_t(data[5]) << 16;
  u.u |= uint64_t(data[6]) << 8;
  u.u |= uint64_t(data[7]);
  HLAfloat64IntervalImpl::setValue(_impl, u.d);
}

std::wstring
HLAfloat64Interval::toString() const
{
  std::wstringstream stream;
  stream << getInterval();
  return stream.str();
}

std::wstring
HLAfloat64Interval::implementationName() const
{
  return L"HLAfloat64Time";
}

double
HLAfloat64Interval::getInterval() const
{
  return HLAfloat64IntervalImpl::getValue(_impl);
}

void
HLAfloat64Interval::setInterval(double value)
{
  HLAfloat64IntervalImpl::setValue(_impl, clearSign(value));
}

HLAfloat64Interval&
HLAfloat64Interval::operator=(const HLAfloat64Interval& value)
{
  HLAfloat64IntervalImpl::assign(_impl, value._impl);
  return *this;
}

HLAfloat64Interval::operator double() const
{
  return getInterval();
}
