#include "limine/requests.h"
#include "modules/loader.h"
#include "subsystems/console/logging.h"
#include "utilities/configparser.h"
#include "utilities/tar.h"

namespace KtKernel
{
    bool LoadCoreModules()
    {
        KtCore::KPrint("Loading core modules.");
        auto* system_tar = Limine::moduleRequest.response->modules[0];
        TarArchive systems_archive;
        systems_archive.open(system_tar->address, system_tar->size);

        auto file = systems_archive.readFile("config.toml");
        if (!file)
            KtCore::Panic("Failed to read config.toml from system archive.");

        TomlParser config;
        if (!config.parse(reinterpret_cast<const char*>(file.m_data), file.m_size))
            KtCore::Panic("Failed to parse config.toml.");

        for (size_t i = 0; i < config.entryCount(); i++) {
            const auto& entry = config.entry(i);
            if (strcmp(entry.m_section, "modules") != 0)
                continue;

            if (entry.m_type != TomlParser::ValueType::Bool || !entry.m_boolVal)
                continue;

            char path[128] = "modules/";
            strcat(path, entry.m_key);
            strcat(path, ".ktdrv");

            KtCore::KPrint("Loading module '{}'...", entry.m_key);
            if (!LoadModule(path))
                KtCore::Panic("Failed to load module '{}'.", entry.m_key);

            KtCore::KPrint("Module '{}' loaded successfully.", entry.m_key);
        }

        return true;
    }
} // namespace KtKernel
