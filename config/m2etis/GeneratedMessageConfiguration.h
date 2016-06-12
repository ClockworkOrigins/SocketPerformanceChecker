/*
 * SocketPerformanceTester
 * Copyright (2016) Daniel Bonrath, Michael Baer, All rights reserved.
 *
 * This file is part of SocketPerformanceTester; SocketPerformanceTester is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef m2etis_MessageConfigurationExample_h
#define m2etis_MessageConfigurationExample_h

#include "m2etis/config/GeneratedEventTypes.h"
#include "m2etis/config/DetMergeConfigTest.h"

#include "boost/serialization/serialization.hpp"
#include "boost/serialization/level.hpp"
#include "boost/serialization/tracking.hpp"

BOOST_CLASS_EXPORT_GUID(m2etis::message::DirectRoutingInfo<m2etis::net::NetworkType<m2etis::net::clockTCP>>, "4")
BOOST_CLASS_IMPLEMENTATION(m2etis::message::DirectRoutingInfo<m2etis::net::NetworkType<m2etis::net::clockTCP>>, boost::serialization::object_serializable)

BOOST_CLASS_EXPORT_GUID(m2etis::message::NullFilterInfo<SPC_EventType>, "5")
BOOST_CLASS_IMPLEMENTATION(m2etis::message::NullFilterInfo<SPC_EventType>, boost::serialization::object_serializable)

BOOST_CLASS_EXPORT_GUID(m2etis::message::NullOrderInfo, "6")
BOOST_CLASS_IMPLEMENTATION(m2etis::message::NullOrderInfo, boost::serialization::object_serializable)

// Direct / Null TCP

typedef m2etis::message::InternalMessage<m2etis::net::NetworkType<m2etis::net::clockTCP>, m2etis::pubsub::ChannelType<
m2etis::pubsub::routing::DirectRouting<m2etis::net::NetworkType<m2etis::net::clockTCP>>
, m2etis::pubsub::filter::NullFilter<SPC_EventType, m2etis::net::NetworkType<m2etis::net::clockTCP> >
, m2etis::pubsub::order::NullOrder<m2etis::net::NetworkType<m2etis::net::clockTCP>>
, m2etis::pubsub::deliver::NullDeliver<m2etis::net::NetworkType<m2etis::net::clockTCP>>
, m2etis::pubsub::persistence::NullPersistence
, m2etis::pubsub::validity::NullValidity
, m2etis::pubsub::partition::NullPartition<m2etis::net::NetworkType<m2etis::net::clockTCP>, SPC_EventType>
, m2etis::pubsub::security::NullSecurity, m2etis::pubsub::rendezvous::NullRendezvous>, SPC_EventType> Direct_Null_Null_Null_Null_Null_Null_Null_String_TCPMessage;
BOOST_CLASS_EXPORT_GUID(Direct_Null_Null_Null_Null_Null_Null_Null_String_TCPMessage, "7")
BOOST_CLASS_IMPLEMENTATION(Direct_Null_Null_Null_Null_Null_Null_Null_String_TCPMessage, boost::serialization::object_serializable)

#endif
