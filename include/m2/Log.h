#ifndef M2_LOG_H
#define M2_LOG_H

#include "Options.h"
#include "M2.h"
#include <string>
#if _MSC_VER > 1400
#include <sal.h>
#endif

/// When to use which debug level:
/// Fatal: Unable to continue, will crash immediately
/// Error: A necessary operation failed, will crash gracefully unless recovered
/// Warn: An optional operation failed, behavior may be unpredictable
/// Info: Minimal indication of expected operation
/// Debug: Everything that is attempted and completed
/// Trace: Like Debug, but for each frame

#define LOGF_TRACE(fmt, ...) ::m2::detail::logf(::m2::pb::LogLevel::TRC, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_DEBUG(fmt, ...) ::m2::detail::logf(::m2::pb::LogLevel::DBG, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_INFO(fmt, ...) ::m2::detail::logf(::m2::pb::LogLevel::INF, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_WARN(fmt, ...) ::m2::detail::logf(::m2::pb::LogLevel::WRN, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_ERROR(fmt, ...) ::m2::detail::logf(::m2::pb::LogLevel::ERR, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_FATAL(fmt, ...) ::m2::detail::logf(::m2::pb::LogLevel::FTL, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)

#define LOG_TRACE(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::TRC, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::DBG, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_INFO(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::INF, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_WARN(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::WRN, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::ERR, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::FTL, __FILE__, __LINE__, (msg), ##__VA_ARGS__)

#define TRACE_FN() LOG_TRACE("f", __FUNCTION__)
#define DEBUG_FN() LOG_DEBUG("f", __FUNCTION__)

namespace m2 {
	void log_stacktrace();

	namespace detail {
		void log_header(pb::LogLevel lvl, const char* file, int line);

#if _MSC_VER > 1400
		void logf(LogLevel lvl, const char* file, int line, _Printf_format_string_ const char* fmt, ...);
#else
		void logf(pb::LogLevel lvl, const char* file, int line, const char* fmt, ...) __attribute__ ((format (printf, 4, 5)));
#endif

		template <typename ...Ts>
		void log(pb::LogLevel lvl, const char* file, int line, const char* msg, const Ts& ...ts) {
			if (lvl < current_log_level) {
				return;
			}
			constexpr std::size_t args_size = std::tuple_size<std::tuple<Ts...>>::value;

			log_header(lvl, file, line);
			fprintf(stderr, args_size ? "%s: " : "%s", msg);
			// Use the argument list of a lambda to unroll the ts
			{
				using namespace std;
				using namespace m2;
				[](...){}((fprintf(stderr, "%s ", to_string(ts).c_str()))...);
			}
			fprintf(stderr, "\n");
		}
	}
}

#endif //M2_LOG_H
