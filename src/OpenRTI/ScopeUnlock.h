/* -*-c++-*- OpenRTI - Copyright (C) 2004-2011 Mathias Froehlich 
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

#ifndef OpenRTI_ScopeUnlock_h
#define OpenRTI_ScopeUnlock_h

#include "Mutex.h"

namespace OpenRTI {

class ScopeUnlock {
public:
  ScopeUnlock(const Mutex& mutex) : mMutex(mutex)
  { mMutex.unlock(); }
  ~ScopeUnlock(void)
  { mMutex.lock(); }

private:
  ScopeUnlock(void);
  ScopeUnlock(const ScopeUnlock&);
  ScopeUnlock& operator=(const ScopeUnlock&);

  const Mutex& mMutex;
};

} // namespace OpenRTI

#endif