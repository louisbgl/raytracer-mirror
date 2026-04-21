#pragma once

#include <unordered_map>
#include <dlfcn.h>
#include <string>

/**
 * @brief Manages dynamic loading of raytracer plugins (.so files).
 *
 * This class provides RAII-style management of plugin shared libraries using libdl.
 * It loads plugins at runtime and provides access to their exported symbols.
 *
 * Plugin Contract (extern "C" ABI):
 * Each plugin must export a factory function with the signature:
 *
 *   extern "C" IInterface* create(<plugin-specific-parameters>);
 *
 * Where:
 * - IInterface is IShape, IMaterial, or ILight
 * - Parameters are plugin-specific (e.g., sphere takes x, y, z, radius, IMaterial*)
 * - The function name is always "create"
 * - Returns a raw pointer; caller (Core/Factory) takes ownership
 */
class PluginLoader {
public:
    PluginLoader() = default;
    ~PluginLoader(); // Will unload all handles

    /**
     * @brief Loads a plugin from the specified path.
     * @param path The file path to the plugin (shared library).
     * @return True if the plugin was loaded successfully, false otherwise.
     * @note Wraps the dlopen function and stores the handle for later use.
     */
    bool load(const std::string& path);

    /**
     * @brief Retrieves a symbol (function) from a loaded plugin.
     * @param pluginPath The file path to the plugin.
     * @param symbolName The name of the symbol to retrieve.
     * @return A pointer to the symbol, or nullptr if not found.
     * @note Wraps the dlsym function and returns the symbol from the appropriate plugin
     */
    void* getSymbol(const std::string& pluginPath, const std::string& symbolName);

    /**
     * @brief Unloads a previously loaded plugin.
     * @param path The file path to the plugin to unload.
     * @note Wraps the dlclose function and removes the handle from the internal map.
     */
    void unload(const std::string& path);

private:
    std::unordered_map<std::string, void*> _handles;
};