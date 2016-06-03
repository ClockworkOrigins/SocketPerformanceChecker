/*
 * SocketPerformanceChecker
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

#ifndef __SPC_PLUGINS_SOCKETPLUGININTERFACE_H__
#define __SPC_PLUGINS_SOCKETPLUGININTERFACE_H__

#include <cstdint>
#include <functional>

#include <qplugin.h>

namespace spc {
namespace plugins {

	#define UDP_BUFFER_SIZE 1024 * 1024 * 10 // 10MB

	/**
	 * \brief interface for plugins used to provide own socket implementations
	 */
	class SocketPluginInterface {
	public:
		virtual ~SocketPluginInterface() {
		}

		/**
		 * \brief returns the name of the plugin as shown in the GUI and used to identify the plugin
		 */
		virtual QString getName() const = 0;

		/**
		 * \brief this methods starts a listen socket to listen to a port used by helper
		 * this method must only return if an error occured or the socket is ready to receive messages
		 * \param[in] port the port this socket shall listen to
		 * \param[in] callback callback to be called whenever a message is received from tester
		 * \return returns true if socket is listening, otherwise false
		 */
		virtual bool listen(uint16_t port, const std::function<void(QString)> & callback) = 0;
		
		/**
		 * \brief creates connection from SocketPerformanceTester to helper tool
		 * \param[in] ip the ip address of the PC the helper tool is running on
		 * \param[in] port the port the helper tool is listening on
		 * \param[in] callback this callback is called for every received message to notify SocketPerformanceTester about a new message to update the GUI
		 * \return returns true if connection was created successfully, otherwise false
		 */
		virtual bool connect(const QString & ip, uint16_t port, const std::function<void(void)> & callback) = 0;
		
		/**
		 * \brief sends a message over the socket
		 * \param[in] message the message to be sent
		 */
		virtual void sendMessage(const QString & message) = 0;

		/**
		 * \brief blocks until either the amount of messages specified or the timeout limit is reached
		 * \param[in] messageCount the amount of messages to wait for
		 * \param[in] timeOut the duration in milliseconds to wait for or -1 to wait an infinite time
		 * \return returns true if all messages were received, false if timeOut occured
		 */
		virtual bool waitForMessages(uint32_t messageCount, int32_t timeOut) = 0;

		/**
		 * \brief disconnects this plugin, has to be set to base state to be able to create a clean new connection
		 */
		virtual void disconnect() = 0;
	};

} /* namespace plugins */
} /* namespace spc */

Q_DECLARE_INTERFACE(spc::plugins::SocketPluginInterface, "spc.plugins.SocketPluginInterface")

#endif /* __SPC_PLUGINS_SOCKETPLUGININTERFACE_H__ */
