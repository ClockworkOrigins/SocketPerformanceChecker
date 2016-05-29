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

#include "BoostUdpPlugin.h"

#include <thread>

#include "boost/bind.hpp"

namespace spc {
namespace plugins {

	BoostUdpSocketPlugin::BoostUdpSocketPlugin() : _buffer(), _ioService(nullptr), _testSocket(nullptr), _helperReceiveCallback(), _checkerReceiveCallback(), _messageCounter(0), _conditionLock(), _conditionVariable(), _targetIP(), _targetPort() {
	}

	bool BoostUdpSocketPlugin::listen(uint16_t port, const std::function<void(QString)> & callback) {
		_helperReceiveCallback = callback;
		_ioService = new boost::asio::io_service();
		_ioService->run();
		_testSocket = new boost::asio::ip::udp::socket(*_ioService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port + 1));
		std::thread([this]() {
			boost::system::error_code error;
			while (!error) {
				boost::asio::ip::udp::endpoint * remoteEndPoint = new boost::asio::ip::udp::endpoint();
				size_t len = _testSocket->receive_from(boost::asio::buffer(_buffer, 1048576), *remoteEndPoint, 0, error);
				handleReceive(error, len, remoteEndPoint);
			}
		}).detach();
		return true;
	}

	bool BoostUdpSocketPlugin::connect(const QString & ip, uint16_t port, const std::function<void(void)> & callback) {
		_checkerReceiveCallback = callback;
		_targetIP = ip.toStdString();
		_targetPort = port + 1;
		_ioService = new boost::asio::io_service();
		_ioService->run();
		_resolver = new boost::asio::ip::udp::resolver(*_ioService);
		_testSocket = new boost::asio::ip::udp::socket(*_ioService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
		std::thread([this]() {
			boost::system::error_code error;
			while (!error) {
				boost::asio::ip::udp::endpoint * remoteEndPoint = new boost::asio::ip::udp::endpoint();
				size_t len = _testSocket->receive_from(boost::asio::buffer(_buffer, 1048576), *remoteEndPoint, 0, error);
				handleReceive(error, len, remoteEndPoint);
			}
		}).detach();
		return true;
	}

	void BoostUdpSocketPlugin::sendMessage(const QString & message) {
		boost::asio::ip::udp::resolver resolver(*_ioService);
		boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), _targetIP, std::to_string(_targetPort));
		boost::asio::ip::udp::endpoint endpoint(*resolver.resolve(query));
		boost::system::error_code err;

		_testSocket->send_to(boost::asio::buffer(message.toStdString()), endpoint);
	}

	void BoostUdpSocketPlugin::waitForMessages(uint32_t messageCount, int32_t timeOut) {
		std::unique_lock<std::mutex> ul(_conditionLock);
		while (_messageCounter < messageCount) {
			_conditionVariable.wait_for(ul, std::chrono::milliseconds(timeOut));
		}
	}

	void BoostUdpSocketPlugin::disconnect() {
		if (_ioService) {
			_ioService->stop();
		}
		if (_testSocket) {
			_testSocket->close();
		}
		delete _testSocket;
		_testSocket = nullptr;
		_messageCounter = 0;
		_helperReceiveCallback = std::function<void(QString)>();
		_checkerReceiveCallback = std::function<void(void)>();
		delete _ioService;
		_ioService = nullptr;
	}

	void BoostUdpSocketPlugin::handleReceive(const boost::system::error_code & e, size_t len, boost::asio::ip::udp::endpoint * remoteEndpoint) {
		if (e.value() != 0) {
			return;
		}
		std::vector<uint8_t> data(len);
		for (size_t i = 0; i < len; i++) {
			data[i] = _buffer[i];
		}
		std::string message(data.begin(), data.begin() + std::string::difference_type(len));

		if (_targetPort == 0) {
			_targetIP = remoteEndpoint->address().to_string();
			_targetPort = remoteEndpoint->port();
		}
		if (_helperReceiveCallback) {
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
