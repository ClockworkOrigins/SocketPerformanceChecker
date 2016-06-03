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

#include "ClockUtilsUdpSyncPlugin.h"

#include "clockUtils/sockets/UdpSocket.h"

namespace spc {
namespace plugins {

	ClockUtilsUdpSocketPlugin::ClockUtilsUdpSocketPlugin() : _testSocket(nullptr), _helperReceiveCallback(), _checkerReceiveCallback(), _messageCounter(0), _conditionLock(), _conditionVariable(), _targetIP(), _targetPort() {
	}

	bool ClockUtilsUdpSocketPlugin::listen(uint16_t port, const std::function<void(QString)> & callback) {
		_helperReceiveCallback = callback;
		_testSocket = new clockUtils::sockets::UdpSocket();
		bool ret = _testSocket->bind(port + 1) == clockUtils::ClockError::SUCCESS;
		if (ret) {
			_testSocket->receiveCallback([this](std::vector<uint8_t> message, std::string ip, uint16_t senderPort, clockUtils::ClockError err) {
				if (err == clockUtils::ClockError::SUCCESS) {
					if (_targetPort == 0) {
						_targetIP = ip;
						_targetPort = senderPort;
					}
					std::string msg(message.begin(), message.end());
					_helperReceiveCallback(QString::fromStdString(msg));
				}
			});
		}
		return ret;
	}

	bool ClockUtilsUdpSocketPlugin::connect(const QString & ip, uint16_t port, const std::function<void(void)> & callback) {
		_checkerReceiveCallback = callback;
		_testSocket = new clockUtils::sockets::UdpSocket();
		bool ret = _testSocket->bind(port) == clockUtils::ClockError::SUCCESS;
		_targetIP = ip.toStdString();
		_targetPort = port + 1;
		if (ret) {
			_testSocket->receiveCallback([this](std::vector<uint8_t>, std::string, uint16_t, clockUtils::ClockError) {
				std::unique_lock<std::mutex> ul(_conditionLock);
				_messageCounter++;
				_conditionVariable.notify_one();
				_checkerReceiveCallback();
			});
		}
		return ret;
	}

	void ClockUtilsUdpSocketPlugin::sendMessage(const QString & message) {
		_testSocket->writePacket(_targetIP, _targetPort, message.toStdString());
	}

	bool ClockUtilsUdpSocketPlugin::waitForMessages(uint32_t messageCount, int32_t timeOut) {
		std::unique_lock<std::mutex> ul(_conditionLock);
		while (_messageCounter < messageCount) {
			if (_conditionVariable.wait_for(ul, std::chrono::milliseconds(timeOut)) == std::cv_status::timeout) {
				break;
			}
		}
		return _messageCounter == messageCount;
	}

	void ClockUtilsUdpSocketPlugin::disconnect() {
		delete _testSocket;
		_testSocket = nullptr;
		_messageCounter = 0;
		_helperReceiveCallback = std::function<void(QString)>();
		_checkerReceiveCallback = std::function<void(void)>();
	}

} /* namespace plugins */
} /* namespace spc */

Q_PLUGIN_METADATA(IID "spc.plugins.SocketPluginInterface")
