#pragma once
#include "Options.h"
#include "M2.h"
#include "protobuf/Detail.h"
#include <mutex>
#include <string>
#if _MSC_VER > 1400
#include <sal.h>
#endif

/// When to use which debug level:
/// Fatal: Unable to continue, will crash immediately
/// Error: A necessary operation failed, behavior may be unpredictable
/// Warn: An optional operation failed, behavior may be unpredictable
/// Info: Minimal indication of expected operation
/// Debug: Everything that is attempted and completed
/// Trace: Like Debug, but for each frame
// TODO it doesn't make sense to have one type of TRACE level. We need to have Network/Game/Physics/etc. types of traces
// TODO and we should be able to selectively enable them. Also we need to keep them quick if they're disabled. For
// TODO example, by putting those calls inside if blocks.

#define LOG_TRACE(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::TRC, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::DBG, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_INFO(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::INF, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_WARN(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::WRN, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::ERR, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) ::m2::detail::Log(::m2::pb::LogLevel::FTL, __FILE__, __LINE__, (msg), ##__VA_ARGS__)

#define LOGF_TRACE(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::TRC, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_DEBUG(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::DBG, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_INFO(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::INF, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_WARN(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::WRN, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_ERROR(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::ERR, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_FATAL(fmt, ...) ::m2::detail::LogF(::m2::pb::LogLevel::FTL, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)

// TODO get rid of these, they are not descriptive enough
#define TRACE_FN() LOG_TRACE("f", __FUNCTION__)
#define DEBUG_FN() LOG_DEBUG("f", __FUNCTION__)
#define INFO_FN() LOG_INFO("f", __FUNCTION__)

namespace m2 {
	// Sets the name of the thread so that it could be used during logging
	void SetThreadNameForLogging(const char* thread_name);
	void LogStacktrace();
	const std::string& ToString(const pb::LogLevel&);

	namespace detail {
		extern std::mutex gLogMutex;

		void LogHeader(pb::LogLevel lvl, const char* file, int line);

		template <typename ...Ts>
		void Log(const pb::LogLevel lvl, const char* file, const int line, const char* msg, const Ts& ...ts) {
			if (lvl < current_log_level) {
				return;
			}
			std::unique_lock lock{gLogMutex};
			LogHeader(lvl, file, line);
			constexpr auto argsSize = std::tuple_size_v<std::tuple<Ts...>>;
			fprintf(stderr, argsSize ? "%s: " : "%s", msg);
			{
				using namespace m2;
				[](...) {}(fprintf(stderr, "%s ", ToString(ts).c_str())...);
			}
			fprintf(stderr, "\n");
		}

#if _MSC_VER > 1400
		void LogF(pb::LogLevel lvl, const char* file, int line, _Printf_format_string_ const char* fmt, ...);
#else
		void LogF(pb::LogLevel lvl, const char* file, int line, const char* fmt, ...) __attribute__ ((format (printf, 4, 5)));
#endif
	}
}
