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

#include "QtTcpPlugin.h"

#include <iostream>

#include <QTcpServer>
#include <QTcpSocket>

namespace spc {
namespace plugins {

	QtTcpSocketPlugin::QtTcpSocketPlugin() : _server(nullptr), _socket(nullptr), _helperReceiveCallback(), _checkerReceiveCallback(), _messageCounter(0), _conditionLock(), _conditionVariable(), _receivedData() {
		QObject::connect(this, SIGNAL(doListen(quint16)), this, SLOT(onListen(quint16)));
		QObject::connect(this, SIGNAL(doConnect(QString, quint16)), this, SLOT(onConnect(QString, quint16)));
		QObject::connect(this, SIGNAL(doSendMessage(QString)), this, SLOT(onSendMessage(QString)));
		QObject::connect(this, SIGNAL(doDisconnect()), this, SLOT(onDisconnect()));
	}

	void QtTcpSocketPlugin::newConnection() {
		if (_server->hasPendingConnections()) {
			_socket = _server->nextPendingConnection();
			QObject::connect(_socket, SIGNAL(readyRead()), this, SLOT(receivedData()));
		}
	}

	void QtTcpSocketPlugin::receivedData() {
		QByteArray data = _socket->readAll();
		_receivedData.append(data);
		QVector<uint8_t> delim = { 0x01, 0x02, 0x03, 0x04, 0x05 };
		QByteArray del;
		for (uint8_t d : delim) {
			del.append(d);
		}
		while (_receivedData.contains(del)) {
			int index = _receivedData.indexOf(del);
			QString message = QString::fromUtf8(_receivedData.mid(0, index + 1));
			_receivedData = _receivedData.mid(index + delim.size());
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

	void QtTcpSocketPlugin::onListen(quint16 port) {
		_server = new QTcpServer(this);
		QObject::connect(_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
		_server->listen(QHostAddress::Any, port);
	}

	void QtTcpSocketPlugin::onConnect(QString ip, quint16 port) {
		_socket = new QTcpSocket(this);
		_socket->connectToHost(ip, port);
		QObject::connect(_socket, SIGNAL(readyRead()), this, SLOT(receivedData()));
	}

	void QtTcpSocketPlugin::onSendMessage(QString message) {
		_socket->write(message.toUtf8());
	}

	void QtTcpSocketPlugin::onDisconnect() {
		if (_server) {
			_server->close();
			_server->deleteLater();
			_server = nullptr;
		}
		if (_socket) {
			_socket->disconnectFromHost();
			_socket->deleteLater();
			_socket = nullptr;
		}
	}

	bool QtTcpSocketPlugin::listen(const QString &, uint16_t port, const std::function<void(QString)> & callback) {
		_helperReceiveCallback = callback;
		emit doListen(port);
		return true;
	}

	bool QtTcpSocketPlugin::connect(const QString & ip, uint16_t port, const QString &, const std::function<void(void)> & callback) {
		_checkerReceiveCallback = callback;
		emit doConnect(ip, port);
		return true;
	}

	void QtTcpSocketPlugin::sendMessage(const QString & message) {
		QString copy = message;
		QVector<uint8_t> delim = { 0x01, 0x02, 0x03, 0x04, 0x05 };
		for (int i = 0; i < delim.size(); ++i) {
			copy.append(delim[i]);
		}
		emit doSendMessage(copy);
	}

	bool QtTcpSocketPlugin::waitForMessages(uint32_t messageCount, int32_t timeOut) {
		std::unique_lock<std::mutex> ul(_conditionLock);
		while (_messageCounter < messageCount) {
			if (_conditionVariable.wait_for(ul, std::chrono::milliseconds(timeOut)) == std::cv_status::timeout) {
				break;
			}
		}
		return _messageCounter == messageCount;
	}

	void QtTcpSocketPlugin::disconnect() {
		emit doDisconnect();
		_messageCounter = 0;
		_helperReceiveCallback = std::function<void(QString)>();
		_checkerReceiveCallback = std::function<void(void)>();
	}

} /* namespace plugins */
} /* namespace spc */

Q_PLUGIN_METADATA(IID "spc.plugins.SocketPluginInterface")
