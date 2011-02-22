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

#ifndef RTI1516Einteger64TimeFactory_h
#define RTI1516Einteger64TimeFactory_h

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>

#include <RTI/time/HLAinteger64Time.h>
#include <RTI/time/HLAinteger64Interval.h>

namespace OpenRTI {

// Interface to build up an optimized OpenRTI::Federate for the builtin
// integer64 logical time

class OPENRTI_LOCAL RTI1516Einteger64TimeFactory {
public:
  typedef int64_t LogicalTime;
  typedef int64_t LogicalTimeInterval;

  RTI1516Einteger64TimeFactory(const std::wstring& name) :
    _name(name)
  { }

  const std::wstring& getName() const
  { return _name; }

  LogicalTime initialLogicalTime() const
  { return LogicalTime(0); }
  LogicalTimeInterval zeroLogicalTimeInterval() const
  { return LogicalTimeInterval(0); }

  LogicalTime getLogicalTime(const rti1516e::LogicalTime& rti1516LogicalTime)
  { _integer64Time = rti1516LogicalTime; return LogicalTime(_integer64Time.getTime()); }
  LogicalTimeInterval getLogicalTimeInterval(const rti1516e::LogicalTimeInterval& rti1516LogicalTimeInterval)
  { _integer64Interval = rti1516LogicalTimeInterval; return LogicalTimeInterval(_integer64Interval.getInterval()); }

  const rti1516e::LogicalTime& getLogicalTime(const LogicalTime& logicalTime)
  { _integer64Time.setTime(logicalTime); return _integer64Time; }
  const rti1516e::LogicalTimeInterval& getLogicalTimeInterval(const LogicalTimeInterval& logicalTimeInterval)
  { _integer64Interval.setInterval(logicalTimeInterval); return _integer64Interval; }

  bool isZeroTimeInterval(const LogicalTimeInterval& logicalTimeInterval)
  { return logicalTimeInterval == 0; }
  bool isZeroTimeInterval(const rti1516e::LogicalTimeInterval& rti1516LogicalTimeInterval)
  { return rti1516LogicalTimeInterval.isZero(); }
  bool isPositiveTimeInterval(const rti1516e::LogicalTimeInterval& rti1516LogicalTimeInterval)
  { _integer64Interval.setInterval(0); return _integer64Interval <= rti1516LogicalTimeInterval; }

  LogicalTime decodeLogicalTime(const VariableLengthData& variableLengthData) const
  {
    if (variableLengthData.size() < 8)
      throw rti1516e::CouldNotDecode(L"Buffer size too short!");
    return LogicalTime(variableLengthData.getInt64BE(0));
  }
  VariableLengthData encodeLogicalTime(const rti1516e::LogicalTime& logicalTime)
  {
    unsigned long encodedLength = logicalTime.encodedLength();
    VariableLengthData variableLengthData;
    variableLengthData.resize(encodedLength);
    logicalTime.encode(variableLengthData.data(), encodedLength);
    return variableLengthData;
  }
  VariableLengthData encodeLogicalTime(const LogicalTime& logicalTime)
  { return encodeLogicalTime(getLogicalTime(logicalTime)); }

  std::wstring toString(const rti1516e::LogicalTime& logicalTime)
  { return logicalTime.toString(); }
  std::wstring toString(const rti1516e::LogicalTimeInterval& logicalTimeInterval)
  { return logicalTimeInterval.toString(); }

private:
  std::wstring _name;
  rti1516e::HLAinteger64Time _integer64Time;
  rti1516e::HLAinteger64Interval _integer64Interval;
};

}

#endif