#pragma once

#include <spdlog/spdlog.h>
#include "Overlay.h"


namespace spdlog {
namespace sinks {

template <typename Mutex>
class overlay_sink : public spdlog::sinks::base_sink<Mutex> {
  public:
    overlay_sink() = default;

  protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        Overlay::AddLog(msg);
    }

    void flush_() override
    {
        // Do nothing because statement executed in sink_it_().
    }

    void set_pattern_(const std::string& pattern) override
    {
        // Don't format log message.
    }

    void set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter) override
    {
        // Don't format log message.
    }
};

using overlay_sink_mt = overlay_sink<std::mutex>;

} // namespace sinks
} // namespace spdlog