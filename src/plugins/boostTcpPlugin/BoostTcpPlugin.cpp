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

#include "BoostTcpPlugin.h"

#include <thread>

namespace spc {
namespace plugins {

	BoostTcpSocketPlugin::BoostTcpSocketPlugin() : _ioService(nullptr), _testSocket(nullptr), _helperReceiveCallback(), _checkerReceiveCallback(), _messageCounter(0), _conditionLock(), _conditionVariable(), _listenThread(nullptr), _receiveThread(nullptr) {
	}

	bool BoostTcpSocketPlugin::listen(uint16_t port, const std::function<void(QString)> & callback) {
		_helperReceiveCallback = callback;
		_ioService = new boost::asio::io_service();
		_ioService->run();
		_resolver = new boost::asio::ip::tcp::resolver(*_ioService);
		_testSocket = new boost::asio::ip::tcp::socket(*_ioService);
		_listenThread = new std::thread([this, port]() {
			boost::asio::ip::tcp::acceptor acceptor(*_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
			acceptor.accept(*_testSocket);
			acceptor.close();
			_receiveThread = new std::thread(std::bind(&BoostTcpSocketPlugin::readFromSocket, this));
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		return true;
	}

	bool BoostTcpSocketPlugin::connect(const QString & ip, uint16_t port, const std::function<void(void)> & callback) {
		_checkerReceiveCallback = callback;
		_ioService = new boost::asio::io_service();
		_ioService->run();
		_resolver = new boost::asio::ip::tcp::resolver(*_ioService);
		_testSocket = new boost::asio::ip::tcp::socket(*_ioService);
		boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), ip.toStdString(), std::to_string(port));
		boost::asio::ip::tcp::resolver::iterator it = _resolver->resolve(query);
		boost::asio::ip::tcp::resolver::iterator end;
		boost::system::error_code error = boost::asio::error::host_not_found;
		_testSocket->connect(*it++, error);
		_receiveThread = new std::thread(std::bind(&BoostTcpSocketPlugin::readFromSocket, this));
		return error == 0;
	}

	void BoostTcpSocketPlugin::sendMessage(const QString & message) {
		std::string str = message.toStdString();
		std::vector<uint8_t> v(str.begin(), str.end());

		std::vector<uint8_t> delim = { 0x01, 0x02, 0x03, 0x04, 0x05 };
		for (size_t i = 0; i < delim.size(); ++i) {
			v.push_back(delim[i]);
		}

		for (size_t i = 0; i < v.size() - delim.size(); ++i) {
			size_t j;
			for (j = 0; j < delim.size(); ++j) {
				if (v[i + j] != delim[j]) {
					break;
				}
			}
			assert(j < delim.size());
		}

		boost::system::error_code err;

		boost::asio::write(*_testSocket, boost::asio::buffer(&(v[0]), v.size()), err);
	}

	bool BoostTcpSocketPlugin::waitForMessages(uint32_t messageCount, int32_t timeOut) {
		std::unique_lock<std::mutex> ul(_conditionLock);
		while (_messageCounter < messageCount) {
			if (_conditionVariable.wait_for(ul, std::chrono::milliseconds(timeOut)) == std::cv_status::timeout) {
				break;
			}
		}
		return _messageCounter == messageCount;
	}

	void BoostTcpSocketPlugin::disconnect() {
		if (_ioService) {
			_ioService->stop();
		}
		if (_testSocket) {
			_testSocket->cancel();
			try {
				_testSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			} catch (boost::system::system_error &) {
			}
			_testSocket->close();
		}
		if (_listenThread) {
			_listenThread->join();
			delete _listenThread;
			_listenThread = nullptr;
		}
		if (_receiveThread) {
			_receiveThread->join();
			delete _receiveThread;
			_receiveThread = nullptr;
		}
		delete _testSocket;
		_testSocket = nullptr;
		_messageCounter = 0;
		_helperReceiveCallback = std::function<void(QString)>();
		_checkerReceiveCallback = std::function<void(void)>();
		delete _resolver;
		_resolver = nullptr;
		delete _ioService;
		_ioService = nullptr;
	}

	void BoostTcpSocketPlugin::readFromSocket() {
		boost::asio::streambuf buf;
		boost::system::error_code error;

		while (!error) {
			// read until message delimiter appears
			char delim[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x00 };
			size_t len = boost::asio::read_until(*_testSocket, buf, delim, error);
			if (error) {
				break;
			} else {
				const char * m = boost::asio::buffer_cast<const char *>(buf.data());
				std::string message(m, m + len - 5);
				buf.consume(len);
				if (_helperReceiveCallback) {
					_helperReceiveCallback(QString::fromStdString(message));
				} else {
					std::unique_lock<std::mutex> ul(_conditionLock);
					_messageCounter++;
					_conditionVariable.notify_one();
					_checkerReceiveCallback();
				}
			}
		}
	}

} /* namespace plugins */
} /* namespace spc */

Q_PLUGIN_METADATA(IID "spc.plugins.SocketPluginInterface")
