//
//  CUNetworkLayer.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides the global initialization (and tear-down) for CUGL network
//  communications. It must be activated before you can access any network connections.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 1/6/23
//
#include <cugl/net/CUNetworkLayer.h>
#include <cugl/util/CUDebug.h>
#include <rtc/rtc.hpp>
#include <variant>
#include <vector>
#include <cstddef>

using namespace cugl::net;

#pragma mark Layer Singleton
/** The RTC network manager singleton */
NetworkLayer* NetworkLayer::_singleton = nullptr;

/**
 * Returns the RTC log level equivalent to a CUGL log level
 *
 * @param level	A CUGL log level
 *
 * @return the RTC log level equivalent to a CUGL log level
 */
static rtc::LogLevel level2rtc(NetworkLayer::Log level) {
	switch (level) {
	case NetworkLayer::Log::NONE:
		return rtc::LogLevel::None;
        case NetworkLayer::Log::FATAL:
		return rtc::LogLevel::Fatal;
	case NetworkLayer::Log::ERRORS:
		return rtc::LogLevel::Error;
	case NetworkLayer::Log::WARNINGS:
    case NetworkLayer::Log::NETCODE:
		return rtc::LogLevel::Warning;
	case NetworkLayer::Log::INFO:
		return rtc::LogLevel::Info;
	case NetworkLayer::Log::DEVELOPER:
		return rtc::LogLevel::Debug;
	case NetworkLayer::Log::VERBOSE:
		return rtc::LogLevel::Verbose;
	}
	return rtc::LogLevel::None;
}

/**
 * Creates the RTC networking layer.
 *
 * @param level	The desired logging level
 *
 * This constructor is a private and should never be access by the user.
 */
NetworkLayer::NetworkLayer(Log level) {
	InitLogger(level2rtc(level));
	rtcPreload();
	_debug = (int)level >= (int)Log::NETCODE;
}

/**
 * Deallocates the RTC network manager, releasing all resources
 */
NetworkLayer::~NetworkLayer() {
	rtcCleanup();
}

/**
 * Starts up the RTC networking layer
 *
 * Once this method is called, the {@link #get} method will no longer return
 * nullptr.  The class {@link NetcodeConnection} require this method before
 * it can properly be used.
 *
 * @param level	The desired logging level
 *
 * @return true if the network sublayer was successfully initialized
 */
bool NetworkLayer::start(Log level) {
	if (_singleton == nullptr) {
		_singleton = new NetworkLayer(level);
	}
    return _singleton != nullptr;
}

/**
 * Shuts down the RTC networking layer
 *
 * Once this method is called, the {@link #get} method will always return nullptr.
 * Any existing instances of {@link NetcodeConnection} will immediately be
 * disconnected, and any further connection attempts will fail.
 *
 * @return true if the network sublayer was successfully shut down
 */
bool NetworkLayer::stop() {
	if (_singleton != nullptr) {
		delete _singleton;
		_singleton = nullptr;
		return true;
	}
	return false;
}
