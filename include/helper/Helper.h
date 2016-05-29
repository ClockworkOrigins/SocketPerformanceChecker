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

#ifndef __SPC_HELPER_H__
#define __SPC_HELPER_H__

#include <cstdint>
#include <map>
#include <vector>

#include <QString>

namespace clockUtils {
	enum class ClockError;
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spc {
namespace plugins {
	class SocketPluginInterface;
} /* namespace plugins */

	class Helper {
	public:
		Helper();
		~Helper();

	private:
		/**
		 * \brief stores all loaded SocketPlugins
		 */
		std::map<std::string, plugins::SocketPluginInterface *> _socketPlugins;

		/**
		 * \brief socket used to handle basic control flow between SocketPerformanceChecker and SocketPerformanceHelper
		 */
		clockUtils::sockets::TcpSocket * _controlSocket;

		/**
		 * \brief stores the currently active plugin
		 */
		plugins::SocketPluginInterface * _activePlugin;

		/**
		 * \brief loads all possible plugin types, actually only SocketPlugins
		 */
		void loadPlugins();

		/**
		 * \brief loads all SocketPlugins
		 */
		void loadSocketPlugins();

		/**
		 * \brief called when a SocketPerformanceChecker connects to this helper
		 */
		void acceptConnection(clockUtils::sockets::TcpSocket * socket);

		/**
		 * \brief called whenever a message from SocketPerformanceChecker itself (not a running test) is received
		 */
		void receivedControlMessage(std::vector<uint8_t> message, clockUtils::sockets::TcpSocket * socket, clockUtils::ClockError err);

		/**
		 * \brief called whenever a message for the real test is received to just echo it
		 */
		void receivedTestMessage(QString message);
	};

} /* namespace spc */

#endif /* __SPC_HELPER_H__ */
