#include "ImGuiLogOutput.h"

namespace MOON
{

ImGuiLogOutput::ImGuiLogOutput(size_t maxEntries)
    : LogOutput("ImGuiLogOutput")
    , m_maxEntries(maxEntries)
    , m_file("moon_imgui_log.txt", std::ios::out | std::ios::app)
{
}

void ImGuiLogOutput::logMessage(Level level, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.emplace_front(Entry{ level, msg });
    if (m_entries.size() > m_maxEntries) {
        m_entries.resize(m_maxEntries);
    }
    if (m_file.is_open()) {
        m_file << msg << '\n';
        m_file.flush();
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
