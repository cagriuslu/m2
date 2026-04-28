#pragma once
#include "Options.h"
#include "ObjectId.h"
#include "M2.h"
#include "protobuf/Detail.h"
#include <mutex>
#include <string>
#include <utility>
#if _MSC_VER > 1400
#include <sal.h>
#endif

/// When to use which debug level:
/// Fatal: Unable to continue, will crash immediately
/// Error: A necessary operation failed, behavior may be unpredictable
/// Warn: An optional operation failed, behavior may be unpredictable
/// Info: Minimal indication of expected operation
/// Debug: Indication of attempted and completed operations (generic)
/// Network: Indication of attempted and completed operations (network related)
/// Physics: Indication of attempted and completed operations (physics related)
/// Graphics: Indication of attempted and completed operations (graphics related)
/// Trace: Indication of attempted and completed operations that happen each frame

// TODO use std::format instead of varargs
#define LOG_TRACE(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::TRC, false, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_GRAPHICS(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::GFX, false, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_PHYSICS(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::PHY, false, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_NETWORK(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::NET, false, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::DBG, false, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_INFO(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::INF, false, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_WARN(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::WRN, false, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::ERR, false, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::FTL, false, __FILE__, __LINE__, (msg), ##__VA_ARGS__)

#define LOG_GRAPHICS_VERBOSE(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::GFX, true, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_PHYSICS_VERBOSE(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::PHY, true, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_NETWORK_VERBOSE(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::NET, true, __FILE__, __LINE__, (msg), ##__VA_ARGS__)

// TODO remove these, use std::format instead
#define LOGF_TRACE(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::TRC, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_GRAPHICS(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::GFX, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_PHYSICS(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::PHY, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_NETWORK(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::NET, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_DEBUG(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::DBG, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_INFO(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::INF, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_WARN(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::WRN, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_ERROR(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::ERR, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_FATAL(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::FTL, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)

#define LOG_OBJECT_DEBUG(id, msg, ...) ::m2::detail::LogObject(__FILE__, __LINE__, (id), (msg), ##__VA_ARGS__)

// TODO get rid of these, they are not descriptive enough
#define TRACE_FN() LOG_TRACE("f", __FUNCTION__)
#define DEBUG_FN() LOG_DEBUG("f", __FUNCTION__)
#define INFO_FN() LOG_INFO("f", __FUNCTION__)

namespace m2 {
	void LogStacktrace();
	const std::string& ToString(const pb::LogLevel&);

	namespace detail {
		extern std::mutex gLogMutex;

		void LogHeader(pb::LogLevel lvl, const char* filePath, int line);

		template <typename Msg, typename ...Ts>
		void Log(const pb::LogLevel lvl, const bool msgIsVerbose, const char* file, const int line, Msg msg, const Ts& ...ts) {
			if (lvl < ::m2::current_log_level || (msgIsVerbose && not ::m2::verbose)) {
				return;
			}
			std::unique_lock lock{gLogMutex};
			LogHeader(lvl, file, line);
			if (constexpr auto argsSize = std::tuple_size_v<std::tuple<Ts...>>) {
				std::cerr << msg << ": ";
				((std::cerr << ::m2::ToString(ts) << " "), ...);
			} else {
				std::cerr << msg;
			}
			std::cerr << std::endl;
		}

#if _MSC_VER > 1400
		void LogF(pb::LogLevel lvl, const char* file, int line, _Printf_format_string_ const char* fmt, ...);
#else
		void LogF(pb::LogLevel lvl, const char* file, int line, const char* fmt, ...) __attribute__ ((format (printf, 4, 5)));
#endif

		bool IsDebugLoggingEnabledForObject(ObjectId);

		template <typename ...Ts>
		void LogObject(const char* file, const int line, const ObjectId id, const char* msg, const Ts& ...ts) {
			if (pb::LogLevel::DBG < current_log_level || not IsDebugLoggingEnabledForObject(id)) {
				return;
			}
			Log(pb::LogLevel::DBG, false, file, line, msg, std::forward<const Ts>(ts)...);
		}
	}
}
