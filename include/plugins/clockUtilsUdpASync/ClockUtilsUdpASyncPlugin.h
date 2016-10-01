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

#ifndef __SPC_PLUGINS_CLOCKUTILSUDPASYNC_H__
#define __SPC_PLUGINS_CLOCKUTILSUDPASYNC_H__

#include "SocketPluginInterface.h"

#include <condition_variable>
#include <mutex>

#include <QObject>

namespace clockUtils {
namespace sockets {
	class UdpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spc {
namespace plugins {

	class ClockUtilsUdpSocketPlugin : public QObject, public plugins::SocketPluginInterface {
		Q_OBJECT
		Q_PLUGIN_METADATA(IID "spc.plugins.SocketPluginInterface")
		Q_INTERFACES(spc::plugins::SocketPluginInterface)

	public:
		ClockUtilsUdpSocketPlugin();

	private:
		/**
		 * \brief the socket used for the test communication
		 */
		clockUtils::sockets::UdpSocket * _testSocket;

		/**
		 * \brief callback to be called for every received message on helper
		 */
		std::function<void(QString)> _helperReceiveCallback;

		/**
		 * \brief callback to be called for every received message on checker
		 */
		std::function<void(void)> _checkerReceiveCallback;

		/**
		 * \brief counts the amount of received messages
		 */
		uint64_t _messageCounter;

		/**
		 * \brief mutex to lock condition variable
		 */
		std::mutex _conditionLock;

		/**
		 * \brief condition variable to wait for all messages to arrive
		 */
		std::condition_variable _conditionVariable;

		/**
		 * \brief stores target IP
		 */
		std::string _targetIP;

		/**
		 * \brief stores target port
		 */
		uint16_t _targetPort;

		/**
		 * \brief returns the name of the plugin as shown in the GUI and used to identify the plugin
		 */
		QString getName() const override {
			return "clockUtils Udp Async";
		}

		/**
		 * \brief this methods starts a listen socket to listen to a port used by helper
		 * this method must only return if an error occured or the socket is ready to receive messages
		 * \param[in] port the port this socket shall listen to
		 * \param[in] callback callback to be called whenever a message is received from tester
		 */
		bool listen(const QString & ip, uint16_t port, const std::function<void(QString)> & callback) override;
		
		/**
		 * \brief creates connection from SocketPerformanceTester to helper tool
		 * \param[in] ip the ip address of the PC the helper tool is running on
		 * \param[in] port the port the helper tool is listening on
		 * \param[in] callback this callback is called for every received message to notify SocketPerformanceTester about a new message to update the GUI
		 */
		bool connect(const QString & ip, uint16_t port, const QString & ownIp, const std::function<void(void)> & callback) override;
		
		/**
		 * \brief sends a message over the socket
		 * \param[in] message the message to be sent
		 */
		void sendMessage(const QString & message) override;

		/**
		 * \brief blocks until either the amount of messages specified or the timeout limit is reached
		 * \param[in] messageCount the amount of messages to wait for
		 * \param[in] timeOut the duration in milliseconds to wait for or -1 to wait an infinite time
		 */
		bool waitForMessages(uint32_t messageCount, int32_t timeOut) override;

		/**
		 * \brief disconnects this plugin, has to be set to base state to be able to create a clean new connection
		 */
		void disconnect() override;
	};

} /* namespace plugins */
} /* namespace spc */

#endif /* __SPC_PLUGINS_CLOCKUTILSUDPASYNC_H__ */
