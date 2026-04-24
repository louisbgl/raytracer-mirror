#pragma once

#include "../plugins/PluginMetadata.hpp"
#include "PluginLoader.hpp"
#include <unordered_map>
#include <vector>
#include <string>

class PluginManager {
public:
    /**
     * @brief Retrieves the singleton instance of the PluginManager.
     * @return A reference to the PluginManager instance.
     */
    static PluginManager& instance();

    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    PluginManager(PluginManager&&) = delete;
    PluginManager& operator=(PluginManager&&) = delete;

    /**
     * @brief Initializes the PluginManager by scanning the plugins directory and loading all plugins.
     */
    void initialize();

    /**
     * @brief Retrieves the create function for a given type name.
     * @param typeName The name of the type for which to retrieve the create function.
     * @return A pointer to the create function, or nullptr if not found.
     */
    void* getCreateFunction(const std::string& typeName) const;

    /**
     * @brief Retrieves the singular form of a given plural name.
     * @param plural The plural name for which to retrieve the singular form.
     * @return A reference to the singular name.
     */
    const std::string& getSingular(const std::string& plural) const;

    /**
     * @brief Retrieves all plugins belonging to a specific category.
     * @param category The category for which to retrieve plugins.
     * @return A vector of PluginMetadata instances.
     */
    std::vector<PluginMetadata> getPluginsByCategory(const std::string& category) const;

private:
    PluginManager() = default;
    ~PluginManager() = default;

    PluginLoader _loader;
    std::unordered_map<std::string, PluginMetadata> _plugins;
    std::unordered_map<std::string, void*> _createFunctions;
    std::unordered_map<std::string, std::string> _pluralToSingular;

    static const std::string CREATE;
    static const std::string METADATA;
    static const std::vector<std::string> CATEGORIES;

    void _scanAndLoadCategory(const std::string& category);
    void _loadPlugin(const std::string& path);
};