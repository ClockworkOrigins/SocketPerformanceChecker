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

#include "MessageStructs.h"

#include <sstream>

namespace spc {
namespace common {

	std::string Message::Serialize() const {
		std::stringstream ss;
		boost::archive::text_oarchive arch(ss, boost::archive::no_header | boost::archive::no_codecvt | boost::archive::no_xml_tag_checking | boost::archive::archive_flags::no_tracking);
		Message * m = const_cast<Message *>(this);
		arch << m;
		return ss.str();
	}

	Message * Message::Deserialize(const std::string & s) {
		std::stringstream ss;
		Message * m;
		ss << s;
		boost::archive::text_iarchive arch(ss, boost::archive::no_header | boost::archive::no_codecvt | boost::archive::no_xml_tag_checking | boost::archive::archive_flags::no_tracking);
		arch >> m;
		return m;
	}

} /* namespace common */
} /* namespace spc */

BOOST_CLASS_EXPORT_GUID(spc::common::Message, "0")
BOOST_CLASS_IMPLEMENTATION(spc::common::Message, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spc::common::ListenOnPluginAndPortMessage, "1")
BOOST_CLASS_IMPLEMENTATION(spc::common::ListenOnPluginAndPortMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spc::common::ListeningMessage, "2")
BOOST_CLASS_IMPLEMENTATION(spc::common::ListeningMessage, boost::serialization::object_serializable)
