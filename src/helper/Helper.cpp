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

#include "Helper.h"

#include "MessageStructs.h"

#include "SocketPluginInterface.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDir>
#include <QPluginLoader>

namespace spc {

	Helper::Helper() : _socketPlugins(), _listenSocket(nullptr), _controlSocket(nullptr), _activePlugin(nullptr) {
		loadPlugins();

		_listenSocket = new clockUtils::sockets::TcpSocket();
		_listenSocket->listen(CONTROL_PORT, 1, true, std::bind(&Helper::acceptConnection, this, std::placeholders::_1));
	}

	Helper::~Helper() {
		delete _listenSocket;
	}

	void Helper::loadPlugins() {
		loadSocketPlugins();
	}

	void Helper::loadSocketPlugins() {
		QDir pluginsDir = QDir(qApp->applicationDirPath() + "/plugins/sockets");
		foreach(QString fileName, pluginsDir.entryList(QDir::Files)) {
			QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
			QObject * plugin = loader.instance();
			if (plugin) {
				plugins::SocketPluginInterface * spi = qobject_cast<plugins::SocketPluginInterface *>(plugin);
				_socketPlugins.insert(std::make_pair(spi->getName().toStdString(), spi));
			}
		}
	}

	void Helper::acceptConnection(clockUtils::sockets::TcpSocket * socket) {
		if (_controlSocket != nullptr) {
			// Still a test running, reject new connection
			delete socket;
			return;
		}
		_controlSocket = socket;
		_controlSocket->receiveCallback(std::bind(&Helper::receivedControlMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	void Helper::receivedControlMessage(std::vector<uint8_t> message, clockUtils::sockets::TcpSocket *, clockUtils::ClockError err) {
		if (err != clockUtils::ClockError::SUCCESS) {
			// if an error occured, close control socket and wait for a new connection
			delete _controlSocket;
			_controlSocket = nullptr;
			return;
		}
		common::Message * msg = common::Message::Deserialize(std::string(message.begin(), message.end()));
		if (msg->type != common::MessageType::LISTEN) {
			// if an error occured, close control socket and wait for a new connection
			delete msg;
			delete _controlSocket;
			_controlSocket = nullptr;
			return;
		}
		common::ListenOnPluginAndPortMessage * lopapm = dynamic_cast<common::ListenOnPluginAndPortMessage *>(msg);
		if (_activePlugin) {
			_activePlugin->disconnect();
		}
		_activePlugin = _socketPlugins[lopapm->pluginName];
		delete msg;
		if (_activePlugin->listen(lopapm->port, std::bind(&Helper::receivedTestMessage, this, std::placeholders::_1))) {
			// no error during listen startup, so inform SocketPerformanceChecker about running helper
			common::ListeningMessage lm;
			_controlSocket->writePacket(lm.Serialize());
		} else {
			// if an error occured, close control socket and wait for a new connection
			delete _controlSocket;
			_controlSocket = nullptr;
			return;
		}
	}

	void Helper::receivedTestMessage(QString message) {
		_activePlugin->sendMessage(message);
	}

} /* namespace spc */
