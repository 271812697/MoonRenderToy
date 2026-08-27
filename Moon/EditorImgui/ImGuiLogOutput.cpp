#include "ImGuiLogOutput.h"

namespace MOON
{

ImGuiLogOutput::ImGuiLogOutput(size_t maxEntries)
    : LogOutput("ImGuiLogOutput")
    , m_maxEntries(maxEntries)
{
}

void ImGuiLogOutput::logMessage(Level level, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.emplace_front(Entry{ level, msg });
    if (m_entries.size() > m_maxEntries) {
        m_entries.resize(m_maxEntries);
    }
}

std::vector<ImGuiLogOutput::Entry> ImGuiLogOutput::GetEntries() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return { m_entries.begin(), m_entries.end() };
}

void ImGuiLogOutput::Clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.clear();
}

} // namespace MOON
