#pragma once

#include <frust_plugin_host/FrustPluginHost.h>

#include "PluginPodWorkspace.h"

class BuiltInPluginHost final
{
public:
    explicit BuiltInPluginHost(PluginPodWorkspace& workspace);
    ~BuiltInPluginHost();

    bool load(juce::String& error);
    bool processCommand(const juce::String& input, juce::String& output);

private:
    PluginPodWorkspace& workspace;
    FrustPluginHandle plugin = nullptr;
};
