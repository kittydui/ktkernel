#include "limine/requests.h"
#include "modules/loader.h"
#include "subsystems/console/logging.h"
#include "utilities/configparser.h"
#include "utilities/tar.h"

namespace kt_kernel
{
    bool load_core_modules()
    {
        print("Loading core modules.");

        auto* system_tar = limine::module_request.response->modules[0];
        tar_archive systems_archive;
        systems_archive.open(system_tar->address, system_tar->size);

        auto file = systems_archive.read_file("config.toml");
        if (!file)
            panic("Failed to read config.toml from system archive.");

        toml_parser config;
        if (!config.parse(reinterpret_cast<const char*>(file.data), file.size))
            panic("Failed to parse config.toml.");

        for (size_t i = 0; i < config.entry_count(); i++) {
            const auto& e = config.get_entry(i);
            if (strcmp(e.section, "modules") != 0)
                continue;

            if (e.type != toml_parser::value_type::bool_val || !e.bool_val)
                continue;

            char path[128] = "modules/";
            strcat(path, e.key);
            strcat(path, ".ktdrv");

            print("Loading module '{}'...", e.key);
            if (!load_module(path))
                panic("Failed to load module '{}'.", e.key);

            print("Module '{}' loaded successfully.", e.key);
        }

        return true;
    }
} // namespace kt_kernel
