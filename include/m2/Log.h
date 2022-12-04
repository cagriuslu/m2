#ifndef M2_LOG_H
#define M2_LOG_H

#include "M2.h"
#include <sstream>
#include <string>

#if _MSC_VER > 1400
#include <sal.h>
#endif

#define LOGF_TRACE(fmt, ...) ::m2::internal::logf(::m2::LogLevel::Trace, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_DEBUG(fmt, ...) ::m2::internal::logf(::m2::LogLevel::Debug, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_INFO(fmt, ...) ::m2::internal::logf(::m2::LogLevel::Info, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_WARN(fmt, ...) ::m2::internal::logf(::m2::LogLevel::Warn, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_ERROR(fmt, ...) ::m2::internal::logf(::m2::LogLevel::Error, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define LOGF_FATAL(fmt, ...) ::m2::internal::logf(::m2::LogLevel::Fatal, __FILE__, __LINE__, (fmt), ##__VA_ARGS__)

#define LOG_TRACE(msg, ...) ::m2::internal::log(::m2::LogLevel::Trace, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) ::m2::internal::log(::m2::LogLevel::Debug, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_INFO(msg, ...) ::m2::internal::log(::m2::LogLevel::Info, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_WARN(msg, ...) ::m2::internal::log(::m2::LogLevel::Warn, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) ::m2::internal::log(::m2::LogLevel::Error, __FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) ::m2::internal::log(::m2::LogLevel::Fatal, __FILE__, __LINE__, (msg), ##__VA_ARGS__)

#define TRACE_FN() ::m2::internal::logf(::m2::LogLevel::Trace, __FILE__, __LINE__, __FUNCTION__)
#define DEBUG_FN() ::m2::internal::logf(::m2::LogLevel::Debug, __FILE__, __LINE__, __FUNCTION__)

namespace m2 {
	enum class LogLevel {
		Trace = 0,
		Debug,
		Info,
		Warn,
		Error,
		Fatal
	};
	std::string to_string(const LogLevel&);

	extern LogLevel current_log_level;
	extern bool unexpected_event_occured;

	void log_stacktrace();

	namespace internal {
		void log_header(LogLevel lvl, const char* file, int line);

#if _MSC_VER > 1400
		void logf(LogLevel lvl, const char* file, int line, _Printf_format_string_ const char* fmt, ...);
#else
		void logf(LogLevel lvl, const char* file, int line, const char* fmt, ...) __attribute__ ((format (printf, 4, 5)));
#endif

		template <typename ...Ts>
		void log(LogLevel lvl, const char* file, int line, const char* msg, const Ts& ...ts) {
			if (lvl < current_log_level) {
				return;
			}
			log_header(lvl, file, line);
			fprintf(stderr, "%s ", msg);
			((void)[&] {
				fprintf(stderr, "%s ", ::m2::to_string(ts).c_str());
			}, ...);
			fprintf(stderr, "\n");
		}
	}
}

#endif //M2_LOG_H
