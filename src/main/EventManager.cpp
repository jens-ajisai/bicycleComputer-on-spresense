/*
 * EventManager.cpp
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

#include "events/EventManager.h"

EventManager::EventManager() {}

int EventManager::processEvent() {
  Event* event;
  int handledCount = 0;

  if (popEvent(&event)) {
    handledCount = sendEvent(event);
    delete event;
  }

  return handledCount;
}


boolean EventManager::removeListener( EventListener* listener ) {
//  mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), listener), mListeners.end());
  return true;
}

boolean EventManager::addListener(int eventCode, EventListener* listener) {
  if (!listener) {
    return false;
  }

  ListenerItem newListener = {listener, eventCode};
  mListeners.push_back(newListener);

  return true;
}


int EventManager::sendEvent(Event* event) {
  if (DEBUG_EVENTS) Log.traceln("*** sendEvent ENTER %s:%s", Event::eventTypeToString(event->getType()), event->getCommandString());
  int handlerCount = 0;

  for (auto & listener : mListeners) 
  {
    if ((listener.callback != 0) &&
        ((listener.eventCode == event->getType()) ||
         (listener.eventCode == Event::kEventAll))) {
      handlerCount++;
      if (DEBUG_EVENTS) Log.verboseln("*** sendEvent HANDLE ENTER");
      (*listener.callback)(event);
      if (DEBUG_EVENTS) Log.verboseln("*** sendEvent HANDLE EXIT");
    }
  }

  if (DEBUG_EVENTS) Log.traceln("*** sendEvent EXIT %s:%s", Event::eventTypeToString(event->getType()), event->getCommandString());

  return handlerCount;
}

boolean EventManager::queueEvent(Event* event) {
  struct mq_attr attr = {EVENTMANAGER_EVENT_QUEUE_SIZE, sizeof(event), 0, 0};
  mqd_t mqfd = mq_open(QUEUE_EVENT, O_CREAT | O_WRONLY| O_NONBLOCK, 0666, &attr);
  if (mqfd == -1) {
    if (DEBUG_EVENTS)
      Log.errorln("EventManager:queueEvent mq_open failed %d %d", mqfd, get_errno());
  } else {
    // Too slow for audio encoding
//    if (DEBUG_EVENTS) Log.traceln("EventManager:queueEvent created message queue. %d", mqfd);
  }

  if (DEBUG_EVENTS)
    Log.errorln("*** queueEvent %d %s:%s", mqfd, Event::eventTypeToString(event->getType()),
                event->getCommandString());

  int ret = mq_send(mqfd, (char*)&event, sizeof(event), 1);
  mq_close(mqfd);
  return (ret > 0);
}

boolean EventManager::popEvent(Event** event) {
  struct mq_attr attr = {EVENTMANAGER_EVENT_QUEUE_SIZE, sizeof(*event), 0, 0};
  mqd_t mqfd = mq_open(QUEUE_EVENT, O_CREAT | O_RDONLY | O_NONBLOCK, 0666, &attr);
  if (mqfd == -1) {
    if (DEBUG_EVENTS) Log.errorln("EventManager:popEvent mq_open failed %d %d", mqfd, get_errno());
  } else {
//    if (DEBUG_EVENTS) Log.traceln("EventManager:popEvent created message queue. %d", mqfd);
  }

  int ret = mq_receive(mqfd, (char*)event, sizeof(*event), NULL);
  mq_close(mqfd);
  return (ret > 0);
}

EventManager eventManager;
