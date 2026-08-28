#pragma once

#include <frust_plugin_host/FrustPluginHost.h>

#include <vector>

#include "PluginPodWorkspace.h"

class ConsolePanel;

class BuiltInPluginHost final
{
public:
    explicit BuiltInPluginHost(PluginPodWorkspace& workspace);
    ~BuiltInPluginHost();

    void attachReplConsole(ConsolePanel& console);
    bool load(juce::String& error);
    bool processFrateCommand(const juce::String& input, juce::String& output);
    bool processReplCommand(const juce::String& input, juce::String& output);
    void registerReplHelpPlugin(const juce::String& name, const juce::String& summary, FrustPluginHandle plugin);
    juce::String getReplBanner() const;
    juce::String getReplPrompt() const;
    juce::String getReplHelp(const juce::String& command) const;

private:
    struct ReplHelpProvider
    {
        juce::String name;
        juce::String summary;
        FrustPluginHandle plugin = nullptr;
    };

    PluginPodWorkspace& workspace;
    ConsolePanel* replConsole = nullptr;
    FrustPluginHandle podPlugin = nullptr;
    FrustPluginHandle replPlugin = nullptr;
    std::vector<ReplHelpProvider> replHelpProviders;
};
