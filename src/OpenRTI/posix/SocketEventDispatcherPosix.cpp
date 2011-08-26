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

#include "SocketEventDispatcher.h"

#if defined __hpux
#include <sys/time.h>
#else
#include <sys/select.h>
#endif
#include <pthread.h>
#include <vector>
#include <map>
#include <cerrno>

#include "AbstractSocketEvent.h"
#include "ClockPosix.h"
#include "Exception.h"
#include "SocketPrivateDataPosix.h"

namespace OpenRTI {

/// FIXME from the linux man page!!!!
/// Rethink!!!!
// Under Linux, select() may report a socket file descriptor as "ready
// for reading", while nevertheless a subsequent read blocks. This
// could for example happen when data has arrived but upon examination
// has wrong checksum and is discarded. There may be other
// circumstances in which a file descriptor is spuriously reported as
// ready. Thus it may be safer to use O_NONBLOCK on sockets that
// should not block.

/// eventfd looks nice too

struct SocketEventDispatcher::PrivateData {
  struct SocketEventSet {
    SharedPtr<AbstractSocketEvent> _socketEvent;
  };

  /// Map from the socket file descriptor to a SocketEvent
  typedef std::map<int,SocketEventSet> SocketEventMap;
  SocketEventMap _socketEventMap;

  bool _done;

  PrivateData() :
    _done(false)
  {
  }

  void insert(const SharedPtr<AbstractSocketEvent>& socketEvent)
  {
    if (!socketEvent.valid())
      return;
    if (!socketEvent->getSocket())
      return;
    int fd = socketEvent->getSocket()->_privateData->_fd;
    _socketEventMap[fd]._socketEvent = socketEvent;
  }

  void erase(const SharedPtr<AbstractSocketEvent>& socketEvent)
  {
    if (!socketEvent.valid())
      return;
    if (!socketEvent->getSocket())
      return;
    int fd = socketEvent->getSocket()->_privateData->_fd;
    SocketEventMap::iterator i = _socketEventMap.find(fd);
    if (i == _socketEventMap.end())
      return;

    _socketEventMap.erase(i);
  }

  int exec(SocketEventDispatcher& dispatcher, const Clock* absclock)
  {
    int retv = 0;

    while (!_done) {
      fd_set readfds;
      fd_set writefds;
      fd_set exceptfds;
      FD_ZERO(&readfds);
      FD_ZERO(&writefds);
      FD_ZERO(&exceptfds);

      int nfds = -1;
      for (SocketEventMap::const_iterator i = _socketEventMap.begin(); i != _socketEventMap.end(); ++i) {
        AbstractSocketEvent* socketEvent = i->second._socketEvent.get();
        if (!socketEvent)
          continue;
        if (socketEvent->getEnableRead()) {
          FD_SET(i->first, &readfds);
          nfds = std::max(nfds, i->first);
        }
        if (socketEvent && socketEvent->getEnableWrite()) {
          FD_SET(i->first, &writefds);
          nfds = std::max(nfds, i->first);
        }
        OpenRTIAssert(i->first == socketEvent->getSocket()->_privateData->_fd);
      }
      // HMM???
      if (nfds == -1) {
        _done = true;
        retv = 0;
        break;
      }

      int count;
      if (absclock) {
        uint64_t now = ClockPosix::now();
        if (absclock->getNSec() < now) {
          count = 0;
          FD_ZERO(&readfds);
          FD_ZERO(&writefds);
          FD_ZERO(&exceptfds);
        } else {
          struct timeval timeval = ClockPosix::toTimeval(absclock->getNSec() - now);
          count = ::select(nfds + 1, &readfds, &writefds, &exceptfds, &timeval);
        }
      } else {
        count = ::select(nfds + 1, &readfds, &writefds, &exceptfds, 0);
      }
      if (count == -1 && errno != EINTR) {
        retv = -1;
        break;
      }
      // Timeout
      if (count == 0) {
        retv = 0;
        break;
      }

      SocketEventMap::const_iterator i = _socketEventMap.begin();
      while (0 < count && i != _socketEventMap.end()) {
        int fd = i->first;
        SharedPtr<AbstractSocketEvent> socketEvent = i->second._socketEvent;
        ++i;
        bool activated = false;
        if (FD_ISSET(fd, &readfds)) {
          dispatcher.read(socketEvent);
          activated = true;
        }
        if (FD_ISSET(fd, &writefds)) {
          dispatcher.write(socketEvent);
          activated = true;
        }
        if (activated)
          --count;
      }
    }

    return retv;
  }
};

SocketEventDispatcher::SocketEventDispatcher() :
  _privateData(new PrivateData)
{
}

SocketEventDispatcher::~SocketEventDispatcher()
{
  delete _privateData;
  _privateData = 0;
}

void
SocketEventDispatcher::setDone(bool done)
{
  _privateData->_done = done;
}

bool
SocketEventDispatcher::getDone() const
{
  return _privateData->_done;
}

void
SocketEventDispatcher::insert(const SharedPtr<AbstractSocketEvent>& socketEvent)
{
  _privateData->insert(socketEvent);
}
void
SocketEventDispatcher::erase(const SharedPtr<AbstractSocketEvent>& socketEvent)
{
  _privateData->erase(socketEvent);
}

int
SocketEventDispatcher::exec()
{
  return _privateData->exec(*this, 0);
}

int
SocketEventDispatcher::exec(const Clock& absclock)
{
  return _privateData->exec(*this, &absclock);
}

}
