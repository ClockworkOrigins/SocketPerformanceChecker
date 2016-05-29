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

#ifndef __SPC_COMMON_MESSAGESTRUCTS_H__
#define __SPC_COMMON_MESSAGESTRUCTS_H__

#include <cstdint>

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/export.hpp"
#include "boost/serialization/map.hpp"

#define CONTROL_PORT 12345

namespace spc {
namespace common {

	enum class MessageType {
		LISTEN,
		LISTENING
	};

	struct Message {
		MessageType type;
		Message() : type() {}
		virtual ~Message() {}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & type;
		}

		std::string Serialize() const;
		static Message * Deserialize(const std::string & s);
	};

	struct ListenOnPluginAndPortMessage : public Message {
		std::string pluginName;
		uint16_t port;

		ListenOnPluginAndPortMessage() : Message(), pluginName(), port() {
			type = MessageType::LISTEN;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & pluginName;
			ar & port;
		}
	};

	struct ListeningMessage : public Message {
		ListeningMessage() : Message() {
			type = MessageType::LISTENING;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
		}
	};

} /* namespace common */
} /* namespace spc */

#endif /* __SPC_COMMON_MESSAGESTRUCTS_H__ */
