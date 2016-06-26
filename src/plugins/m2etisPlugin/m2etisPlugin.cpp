/*
 * SocketPerformanceTester
 * Copyright (2016) Daniel Bonrath, Michael Baer, All rights reserved.
 *
 * This file is part of i6engine; i6engine is free software; you can redistribute it and/or
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

#include "m2etisPlugin.h"

#include <thread>

#include "clockUtils/sockets/TcpSocket.h"

#include "m2etis/pubsub/PubSubSystem.h"

namespace spc {
namespace plugins {

	m2etisSocketPlugin::m2etisSocketPlugin() : _pubSubSystem(nullptr), _helperReceiveCallback(), _checkerReceiveCallback(), _messageCounter(0), _conditionLock(), _conditionVariable() {
	}

	bool m2etisSocketPlugin::listen(const QString & ip, uint16_t port, const std::function<void(QString)> & callback) {
		_pubSubSystem = new m2etis::pubsub::PubSubSystem(ip.toStdString(), port, ip.toStdString(), port, { ip.toStdString() });
		_helperReceiveCallback = callback;
		return true;
	}

	bool m2etisSocketPlugin::connect(const QString & ip, uint16_t port, const QString & ownIp, const std::function<void(void)> & callback) {
		_pubSubSystem = new m2etis::pubsub::PubSubSystem(ownIp.toStdString(), port + 1, ip.toStdString(), port, { ip.toStdString() });
		_pubSubSystem->subscribe(m2etis::pubsub::ChannelName::SPC_Direct_Null_Null_Null_Null_Null_Null_Null_String_TCP, *this);
		_checkerReceiveCallback = callback;
		return true;
	}

	void m2etisSocketPlugin::sendMessage(const QString & message) {
		std::string str = message.toStdString();
		std::vector<uint8_t> v(str.begin(), str.end());
		m2etis::message::M2etisMessage::Ptr m = _pubSubSystem->createMessage<SPC_EventType>(m2etis::pubsub::ChannelName::SPC_Direct_Null_Null_Null_Null_Null_Null_Null_String_TCP, v);
		_pubSubSystem->publish<SPC_EventType>(m2etis::pubsub::ChannelName::SPC_Direct_Null_Null_Null_Null_Null_Null_Null_String_TCP, m);
	}

	bool m2etisSocketPlugin::waitForMessages(uint32_t messageCount, int32_t timeOut) {
		std::unique_lock<std::mutex> ul(_conditionLock);
		while (_messageCounter < messageCount) {
			if (_conditionVariable.wait_for(ul, std::chrono::milliseconds(timeOut)) == std::cv_status::timeout) {
				break;
			}
		}
		return _messageCounter == messageCount;
	}

	void m2etisSocketPlugin::disconnect() {
		delete _pubSubSystem;
		_messageCounter = 0;
		_helperReceiveCallback = std::function<void(QString)>();
		_checkerReceiveCallback = std::function<void(void)>();
	}

	void m2etisSocketPlugin::deliverCallback(const m2etis::message::M2Message<SPC_EventType>::Ptr m) {
		if (_helperReceiveCallback) {
			std::string message(m->payload->begin(), m->payload->end());
			_helperReceiveCallback(QString::fromStdString(message));
		} else {
			std::unique_lock<std::mutex> ul(_conditionLock);
			_messageCounter++;
			_conditionVariable.notify_one();
			_checkerReceiveCallback();
		}
	}

} /* namespace plugins */
} /* namespace spc */

Q_PLUGIN_METADATA(IID "spc.plugins.SocketPluginInterface")
