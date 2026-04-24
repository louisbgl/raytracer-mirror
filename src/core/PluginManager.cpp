#include "PluginManager.hpp"
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <dlfcn.h>

// This is to make Intellisense shut up.
// CMake defines it either way.
#ifndef PLUGIN_EXTENSION
    #define PLUGIN_EXTENSION ".so"
#endif

const std::string PluginManager::CREATE = "create";
const std::string PluginManager::METADATA = "metadata";
const std::vector<std::string> PluginManager::CATEGORIES = {"shapes", "materials", "lights"};

PluginManager& PluginManager::instance() {
    static PluginManager instance;
    return instance;
}

void PluginManager::initialize() {
    for (const auto& category : CATEGORIES) {
        _scanAndLoadCategory(category);
    }
}

void* PluginManager::getCreateFunction(const std::string& typeName) const {
    auto it = _createFunctions.find(typeName);
    if (it != _createFunctions.end()) {
        return it->second;
    }
    return nullptr;
}

const std::string& PluginManager::getSingular(const std::string& plural) const {
    auto it = _pluralToSingular.find(plural);
    if (it != _pluralToSingular.end()) {
        return it->second;
    }
    return plural;
}

std::vector<PluginMetadata> PluginManager::getPluginsByCategory(const std::string& category) const {
    std::vector<PluginMetadata> result;
    for (const auto& [name, meta] : _plugins) {
        if (meta.category == category) {
            result.push_back(meta);
        }
    }

    std::sort(result.begin(), result.end(), [](const PluginMetadata& a, const PluginMetadata& b) {
        return std::string(a.pluginName) < std::string(b.pluginName);
    });

    return result;
}

void PluginManager::_scanAndLoadCategory(const std::string& category) {
    std::string path = "./plugins/" + category + "/";

    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            // PLUGIN_EXTENSION is defined by CMake as ".so" or ".dlsym"
            if (entry.path().extension() != PLUGIN_EXTENSION) {
                continue;
            }
            _loadPlugin(entry.path().string());
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Error scanning " << path << ": " << ex.what() << std::endl;
    }
}

void PluginManager::_loadPlugin(const std::string& path) {
    if (!_loader.load(path)) {
        std::cerr << "Failed to load plugin: " << path << std::endl;
        std::cerr << "  dlopen error: " << dlerror() << std::endl;
        return;
    }

    void* metadataSym = _loader.getSymbol(path, METADATA);
    if (!metadataSym) {
        std::cerr << "Failed to find " << METADATA << "() function in plugin: " << path << ".\nThis plugin won't be loaded." << std::endl;
        _loader.unload(path);
        return;
    }

    void* createSym = _loader.getSymbol(path, CREATE);
    if (!createSym) {
        std::cerr << "Failed to find " << CREATE << "() function in plugin: " << path << ".\nThis plugin won't be loaded." << std::endl;
        _loader.unload(path);
        return;
    }

    auto metaFunc = reinterpret_cast<const PluginMetadata*(*)()>(metadataSym);
    const PluginMetadata* meta = metaFunc();

    _plugins[meta->pluginName] = *meta;
    _pluralToSingular[meta->pluralForm] = meta->pluginName;
    _createFunctions[meta->pluginName] = createSym;
}