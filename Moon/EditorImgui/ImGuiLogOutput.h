#pragma once

#include "core/logOutput.h"

#include <deque>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace MOON
{

/** LogOutput implementation that stores messages in a bounded ring buffer.
 *
 * Registered into MOON::Log so that all LOG_* messages become visible to the
 * ImGui editor log panel. The buffer is thread-safe because log messages may
 * arrive from worker threads (e.g. JobSystem).
 */
class ImGuiLogOutput final : public LogOutput
{
public:
    struct Entry
    {
        LogOutput::Level level = LogOutput::LL_INFO;
        std::string message;
    };

    explicit ImGuiLogOutput(size_t maxEntries = 512);

    void logMessage(Level level, const std::string& msg) override;

    /// Copy of the buffered messages (newest first).
    std::vector<Entry> GetEntries() const;

    /// Clear the ring buffer.
    void Clear();

private:
    mutable std::mutex m_mutex;
    std::deque<Entry> m_entries;
    size_t m_maxEntries;
};

} // namespace MOON
