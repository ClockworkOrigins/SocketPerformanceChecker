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

#include "QtUdpPlugin.h"

#include <iostream>

#include <QUdpSocket>

namespace spc {
namespace plugins {

	QtUdpSocketPlugin::QtUdpSocketPlugin() : _socket(nullptr), _helperReceiveCallback(), _checkerReceiveCallback(), _messageCounter(0), _conditionLock(), _conditionVariable(), _targetHost(), _targetPort(0) {
		QObject::connect(this, SIGNAL(doListen(quint16)), this, SLOT(onListen(quint16)));
		QObject::connect(this, SIGNAL(doConnect(QString, quint16)), this, SLOT(onConnect(QString, quint16)));
		QObject::connect(this, SIGNAL(doSendMessage(QString)), this, SLOT(onSendMessage(QString)));
		QObject::connect(this, SIGNAL(doDisconnect()), this, SLOT(onDisconnect()));
	}

	void QtUdpSocketPlugin::receivedData() {
		while (_socket->hasPendingDatagrams()) {
			QByteArray buffer;
			buffer.resize(_socket->pendingDatagramSize());

			// qint64 QUdpSocket::readDatagram(char * data, qint64 maxSize, 
			//                 QHostAddress * address = 0, quint16 * port = 0)
			// Receives a datagram no larger than maxSize bytes and stores it in data. 
			// The sender's host address and port is stored in *address and *port 
			// (unless the pointers are 0).

			_socket->readDatagram(buffer.data(), buffer.size(), &_targetHost, &_targetPort);
			QString message = QString::fromUtf8(buffer);
			if (_helperReceiveCallback) {
				_helperReceiveCallback(message);
			} else {
				std::unique_lock<std::mutex> ul(_conditionLock);
				_messageCounter++;
				_conditionVariable.notify_one();
				_checkerReceiveCallback();
			}
		}
	}

	void QtUdpSocketPlugin::onListen(quint16 port) {
		_socket = new QUdpSocket(this);
		_socket->setReadBufferSize(UDP_BUFFER_SIZE);
		QObject::connect(_socket, SIGNAL(readyRead()), this, SLOT(receivedData()));
		_socket->bind(QHostAddress::Any, port + 1);
	}

	void QtUdpSocketPlugin::onConnect(QString ip, quint16 port) {
		_socket = new QUdpSocket(this);
		_socket->setReadBufferSize(UDP_BUFFER_SIZE);
		_socket->bind(QHostAddress::Any, port);
		_targetHost = ip;
		_targetPort = port + 1;
		QObject::connect(_socket, SIGNAL(readyRead()), this, SLOT(receivedData()));
	}

	void QtUdpSocketPlugin::onSendMessage(QString message) {
		_socket->writeDatagram(message.toUtf8(), _targetHost, _targetPort);
	}

	void QtUdpSocketPlugin::onDisconnect() {
		if (_socket) {
			_socket->disconnectFromHost();
			_socket->deleteLater();
			_socket = nullptr;
		}
	}

	bool QtUdpSocketPlugin::listen(const QString &, uint16_t port, const std::function<void(QString)> & callback) {
		_helperReceiveCallback = callback;
		emit doListen(port);
		return true;
	}

	bool QtUdpSocketPlugin::connect(const QString & ip, uint16_t port, const QString &, const std::function<void(void)> & callback) {
		_checkerReceiveCallback = callback;
		emit doConnect(ip, port);
		return true;
	}

	void QtUdpSocketPlugin::sendMessage(const QString & message) {
		emit doSendMessage(message);
	}

	bool QtUdpSocketPlugin::waitForMessages(uint32_t messageCount, int32_t timeOut) {
		std::unique_lock<std::mutex> ul(_conditionLock);
		while (_messageCounter < messageCount) {
			if (_conditionVariable.wait_for(ul, std::chrono::milliseconds(timeOut)) == std::cv_status::timeout) {
				break;
			}
		}
		return _messageCounter == messageCount;
	}

	void QtUdpSocketPlugin::disconnect() {
		emit doDisconnect();
		_messageCounter = 0;
		_helperReceiveCallback = std::function<void(QString)>();
		_checkerReceiveCallback = std::function<void(void)>();
	}

} /* namespace plugins */
} /* namespace spc */

Q_PLUGIN_METADATA(IID "spc.plugins.SocketPluginInterface")
