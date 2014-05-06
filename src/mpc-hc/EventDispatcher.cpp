/*
 * (C) 2013-2014 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "EventDispatcher.h"

EventRouter::EventRouterCore::EventRouterCore()
    : m_tid(GetCurrentThreadId())
    , m_bDestroyed(false)
{
}

void EventRouter::EventRouterCore::FireEvent(MpcEvent ev)
{
    for (const auto& pair : m_conns) {
        if (pair.second.receives.find(ev) != std::end(pair.second.receives)) {
            pair.second.callback(ev);
        }
    }
}

EventRouter::EventRouter()
    : m_core(std::make_shared<EventRouterCore>())
{
}

EventRouter::~EventRouter()
{
    m_core->m_bDestroyed = true;
}

void EventRouter::Connect(EventClient& node, const EventSelection& receives, const EventCallback& callback)
{
    ASSERT(GetCurrentThreadId() == m_core->m_tid);
    EventRouterCore::EventClientInfo info;
    info.receives = receives;
    info.callback = callback;
    m_core->m_conns[&node] = info;
    node.m_conns.insert(m_core);
}

void EventRouter::Connect(EventClient& node, const EventSelection& fires)
{
    ASSERT(GetCurrentThreadId() == m_core->m_tid);
    EventRouterCore::EventClientInfo info;
    info.fires = fires;
    m_core->m_conns[&node] = info;
    node.m_conns.insert(m_core);
}

void EventRouter::Connect(EventClient& node, const EventSelection& receives, const EventCallback& callback, const EventSelection& fires)
{
    ASSERT(GetCurrentThreadId() == m_core->m_tid);
    EventRouterCore::EventClientInfo info;
    info.receives = receives;
    info.fires = fires;
    info.callback = callback;
    m_core->m_conns[&node] = info;
    node.m_conns.insert(m_core);
}

void EventRouter::Disconnect(EventClient& node)
{
    ASSERT(GetCurrentThreadId() == m_core->m_tid);
    m_core->m_conns.erase(&node);
    node.m_conns.erase(m_core);
}

void EventClient::FireEvent(MpcEvent ev)
{
    for (const auto& conn : m_conns) {
        ASSERT(GetCurrentThreadId() == conn->m_tid);
        if (!conn->m_bDestroyed) {
            ASSERT(conn->m_conns[this].fires.find(ev) != std::end(conn->m_conns[this].fires));
            conn->FireEvent(ev);
        }
    }
}

EventClient::~EventClient()
{
    for (const auto& conn : m_conns) {
        ASSERT(GetCurrentThreadId() == conn->m_tid);
        conn->m_conns.erase(this);
    }
}
