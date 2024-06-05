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

#define LOG_TRACE(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::TRC, __FILE__, __LINE__, {}, (msg), ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::DBG, __FILE__, __LINE__, {}, (msg), ##__VA_ARGS__)
#define LOG_INFO(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::INF, __FILE__, __LINE__, {}, (msg), ##__VA_ARGS__)
#define LOG_WARN(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::WRN, __FILE__, __LINE__, {}, (msg), ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::ERR, __FILE__, __LINE__, {}, (msg), ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::FTL, __FILE__, __LINE__, {}, (msg), ##__VA_ARGS__)

#define LOG_TRACE_INC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::TRC, __FILE__, __LINE__, true, (msg), ##__VA_ARGS__)
#define LOG_DEBUG_INC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::DBG, __FILE__, __LINE__, true, (msg), ##__VA_ARGS__)
#define LOG_INFO_INC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::INF, __FILE__, __LINE__, true, (msg), ##__VA_ARGS__)
#define LOG_WARN_INC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::WRN, __FILE__, __LINE__, true, (msg), ##__VA_ARGS__)
#define LOG_ERROR_INC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::ERR, __FILE__, __LINE__, true, (msg), ##__VA_ARGS__)
#define LOG_FATAL_INC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::FTL, __FILE__, __LINE__, true, (msg), ##__VA_ARGS__)

#define LOG_TRACE_DEC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::TRC, __FILE__, __LINE__, false, (msg), ##__VA_ARGS__)
#define LOG_DEBUG_DEC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::DBG, __FILE__, __LINE__, false, (msg), ##__VA_ARGS__)
#define LOG_INFO_DEC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::INF, __FILE__, __LINE__, false, (msg), ##__VA_ARGS__)
#define LOG_WARN_DEC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::WRN, __FILE__, __LINE__, false, (msg), ##__VA_ARGS__)
#define LOG_ERROR_DEC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::ERR, __FILE__, __LINE__, false, (msg), ##__VA_ARGS__)
#define LOG_FATAL_DEC(msg, ...) ::m2::detail::log(::m2::pb::LogLevel::FTL, __FILE__, __LINE__, false, (msg), ##__VA_ARGS__)

#define TRACE_FN() LOG_TRACE("f", __FUNCTION__)
#define DEBUG_FN() LOG_DEBUG("f", __FUNCTION__)
#define INFO_FN() LOG_INFO("f", __FUNCTION__)

namespace m2 {
	void init_thread_logger(const char* thread_name);

	void log_stacktrace();

	const std::string& to_string(const pb::LogLevel&);

	namespace detail {
		extern std::mutex log_mutex;
		extern thread_local int thread_indentation;

		void log_header(pb::LogLevel lvl, const char* file, int line);

#if _MSC_VER > 1400
		void logf(pb::LogLevel lvl, const char* file, int line, _Printf_format_string_ const char* fmt, ...);
#else
		void logf(pb::LogLevel lvl, const char* file, int line, const char* fmt, ...) __attribute__ ((format (printf, 4, 5)));
#endif

		template <typename ...Ts>
		void log(pb::LogLevel lvl, const char* file, int line, std::optional<bool> indentation, const char* msg, const Ts& ...ts) {
			if (lvl < current_log_level) {
				return;
			}

			std::unique_lock<std::mutex> lock{log_mutex};

			log_header(lvl, file, line);
			m2_repeat(thread_indentation) {
				fprintf(stderr, "  ");
			}

			constexpr auto args_size = std::tuple_size<std::tuple<Ts...>>::value;
			fprintf(stderr, args_size ? "%s: " : "%s", msg);
			// Use the argument list of a lambda to unroll the ts
			{
				using namespace m2;
				[](...){}(fprintf(stderr, "%s ", to_string(ts).c_str())...);
			}
			fprintf(stderr, "\n");

			// Change indentation
			if (indentation) {
				if (*indentation) {
					thread_indentation++;
				} else {
					thread_indentation = std::max(--thread_indentation, 0);
				}
			}
		}
	}
}
