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

#include "ClockUtilsTcpASyncPlugin.h"

#include "clockUtils/sockets/TcpSocket.h"

namespace spc {
namespace plugins {

	ClockUtilsTcpSocketPlugin::ClockUtilsTcpSocketPlugin() : _listenSocket(nullptr), _testSocket(nullptr), _helperReceiveCallback(), _checkerReceiveCallback(), _messageCounter(0), _conditionLock(), _conditionVariable() {
	}

	bool ClockUtilsTcpSocketPlugin::listen(uint16_t port, const std::function<void(QString)> & callback) {
		_helperReceiveCallback = callback;
		_listenSocket = new clockUtils::sockets::TcpSocket();
		
		return _listenSocket->listen(port, 1, false, [this](clockUtils::sockets::TcpSocket * socket) {
			_testSocket = socket;
			_testSocket->receiveCallback([this](std::vector<uint8_t> message, clockUtils::sockets::TcpSocket *, clockUtils::ClockError err) {
				if (err == clockUtils::ClockError::SUCCESS) {
					std::string msg(message.begin(), message.end());
					_helperReceiveCallback(QString::fromStdString(msg));
				}
			});
		}) == clockUtils::ClockError::SUCCESS;
	}

	bool ClockUtilsTcpSocketPlugin::connect(const QString & ip, uint16_t port, const std::function<void(void)> & callback) {
		_checkerReceiveCallback = callback;
		_testSocket = new clockUtils::sockets::TcpSocket();
		bool ret = _testSocket->connect(ip.toStdString(), port, 1000) == clockUtils::ClockError::SUCCESS;
		if (ret) {
			_testSocket->receiveCallback([this](std::vector<uint8_t>, clockUtils::sockets::TcpSocket *, clockUtils::ClockError) {
				std::unique_lock<std::mutex> ul(_conditionLock);
				_messageCounter++;
				_conditionVariable.notify_one();
				_checkerReceiveCallback();
			});
		}
		return ret;
	}

	void ClockUtilsTcpSocketPlugin::sendMessage(const QString & message) {
		_testSocket->writePacketAsync(message.toStdString());
	}

	void ClockUtilsTcpSocketPlugin::waitForMessages(uint32_t messageCount, int32_t timeOut) {
		std::unique_lock<std::mutex> ul(_conditionLock);
		while (_messageCounter < messageCount) {
			_conditionVariable.wait_for(ul, std::chrono::milliseconds(timeOut));
		}
	}

	void ClockUtilsTcpSocketPlugin::disconnect() {
		delete _listenSocket;
		_listenSocket = nullptr;
		delete _testSocket;
		_testSocket = nullptr;
		_messageCounter = 0;
		_helperReceiveCallback = std::function<void(QString)>();
		_checkerReceiveCallback = std::function<void(void)>();
	}

} /* namespace plugins */
} /* namespace spc */

Q_PLUGIN_METADATA(IID "spc.plugins.SocketPluginInterface")
