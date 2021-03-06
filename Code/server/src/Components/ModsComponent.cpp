#include <Components.h>

uint32_t ModsComponent::AddStandard(const char* acpFilename) noexcept
{
    const auto itor = m_standardMods.find(acpFilename);
    if (itor != std::end(m_standardMods))
    {
        itor->second.refCount++;
        return itor->second.id;
    }

    const auto id = m_seed++;
    m_standardMods.emplace(acpFilename, Entry{ id, 1 });

    return id;
}

uint32_t ModsComponent::AddLite(const char* acpFilename) noexcept
{
    const auto itor = m_liteMods.find(acpFilename);
    if (itor != std::end(m_liteMods))
    {
        itor->second.refCount++;
        return itor->second.id;
    }

    const auto id = m_seed++;
    m_liteMods.emplace(acpFilename, Entry{ id, 1 });

    return id;
}
