#include "PluginLoader.hpp"

PluginLoader::~PluginLoader() {
    for (const auto& pair : _handles) {
        dlclose(pair.second);
    }
}

bool PluginLoader::load(const std::string& path) {
    if (_handles.find(path) != _handles.end()) return true;

    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) return false;
    _handles[path] = handle;
    return true;
}

void* PluginLoader::getSymbol(const std::string& pluginPath, const std::string& symbolName) {
    auto it = _handles.find(pluginPath);
    if (it == _handles.end()) return nullptr;

    void* symbol = dlsym(it->second, symbolName.c_str());
    return symbol;
}

void PluginLoader::unload(const std::string& path) {
    auto it = _handles.find(path);
    if (it != _handles.end()) {
        dlclose(it->second);
        _handles.erase(it);
    }
}