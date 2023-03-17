/*
 * EventManager.h
 *

// Modified for this project. Original license is

 * An event handling system for Arduino.
 *
 * Author: igormt@alumni.caltech.edu
 * Copyright (c) 2013 Igor Mikolic-Torreira
 *
 * Inspired by and adapted from the
 * Arduino Event System library by
 * Author: mromani@ottotecnica.com
 * Copyright (c) 2010 OTTOTECNICA Italy
 *
 * This library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser
 * General Public License along with this library; if not,
 * write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#ifndef EventManager_h
#define EventManager_h

  #include <fcntl.h>
#include <nuttx/mqueue.h>

#include <algorithm>
#include <vector>

#include "Event.h"

#ifndef EVENTMANAGER_EVENT_QUEUE_SIZE
#define EVENTMANAGER_EVENT_QUEUE_SIZE 16
#endif

class Event;

class EventListener {
 public:
  virtual void operator()(Event* event) = 0;
};

class EventManager {
 public:
  // Create an event manager
  // It always operates in interrupt safe mode, allowing you to queue events from interrupt handlers
  EventManager();

  boolean addListener(int eventCode, EventListener* listener);
  boolean removeListener(EventListener* listener);
  boolean queueEvent(Event* event);
  int processEvent();

 private:
  int sendEvent(Event* event);
  boolean popEvent(Event** event);

  struct ListenerItem {
    EventListener* callback;
    int eventCode;
  };
  std::vector<ListenerItem> mListeners;
};

extern EventManager eventManager;

#endif
