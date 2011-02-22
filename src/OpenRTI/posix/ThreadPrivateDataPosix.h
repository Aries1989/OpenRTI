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

#ifndef OpenRTI_ThreadPrivateDataPosix_h
#define OpenRTI_ThreadPrivateDataPosix_h

#include <pthread.h>

#include "Export.h"
#include "SharedPtr.h"
#include "Thread.h"

namespace OpenRTI {

struct OPENRTI_LOCAL Thread::PrivateData {
  PrivateData() :
    _started(false)
  {
  }
  ~PrivateData()
  {
    // If we are still having a started thread and nobody waited,
    // now detach ...
    if (!_started)
      return;
    pthread_detach(_thread);
  }

  static void *start_routine(void* data)
  {
    SharedPtr<Thread> thread = reinterpret_cast<Thread*>(data);
    thread->run();
    return 0;
  }

  bool start(Thread& thread)
  {
    if (_started)
      return false;
    if (0 != pthread_create(&_thread, 0, start_routine, &thread))
      return false;
    _started = true;
#ifdef HAVE_PTHREAD_SETNAME_NP
    // Ignore errors since this is just a debugging aid - nothing more
    pthread_setname_np(_thread, thread.getName().c_str());
#endif
    return true;
  }

  void wait()
  {
    if (!_started)
      return;
    pthread_join(_thread, 0);
    _started = false;
  }

  pthread_t _thread;
  bool _started;
};

} // namespace OpenRTI

#endif