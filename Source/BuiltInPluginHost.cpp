#include "BuiltInPluginHost.h"

namespace
{
PluginPodWorkspace* activeWorkspace = nullptr;

extern "C" int64_t suite_create_plugin_pod(const char* name, const char* target)
{
    return activeWorkspace != nullptr && name != nullptr
        && activeWorkspace->createPluginPod(name, target != nullptr ? target : "suite") ? 1 : 0;
}
}

BuiltInPluginHost::BuiltInPluginHost(PluginPodWorkspace& workspaceToUse)
    : workspace(workspaceToUse)
{
}

BuiltInPluginHost::~BuiltInPluginHost()
{
    if (plugin != nullptr)
        frust_plugin_unload(plugin);
}

bool BuiltInPluginHost::load(juce::String& error)
{
    activeWorkspace = &workspace;
    frust_plugin_host_set_application_identity("creation-developer");
    frust_plugin_register_host_function("suite_create_plugin_pod", reinterpret_cast<void*>(&suite_create_plugin_pod));

    const auto pluginFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                                .getParentDirectory().getChildFile("plugins").getChildFile("SuitePluginPodCommands.frust");
    if (! pluginFile.existsAsFile())
    {
        error = "Built-in plugin source is missing: " + pluginFile.getFullPathName();
        return false;
    }

    plugin = frust_plugin_load(pluginFile.getFullPathName().toRawUTF8());
    if (plugin == nullptr)
    {
        error = juce::String(frust_plugin_last_error());
        return false;
    }

    frust_plugin_call_on_init(plugin);
    return true;
}

bool BuiltInPluginHost::processFrateCommand(const juce::String& input, juce::String& output)
{
    auto words = juce::StringArray::fromTokens(input.trim(), " \t\r\n", "");
    if (words.isEmpty())
        return false;

    if (words.size() >= 2 && words[0].equalsIgnoreCase("new") && words[1].equalsIgnoreCase("pod"))
        words.remove(1);

    if ((words.size() != 2 && words.size() != 4) || ! words[0].equalsIgnoreCase("new")
        || (words.size() == 4 && words[2] != "--for"))
    {
        output = "Suite FRust terminal supports: frate new <plugin-pod-name> [--for suite|station|engine|movie|live|texture|modeler|developer]";
        return true;
    }

    if (plugin == nullptr)
    {
        output = "The Suite Plugin Pod Commands plugin is not loaded.";
        return true;
    }

    auto createPod = reinterpret_cast<int64_t (*)(const char*, const char*)>(frust_plugin_get_fn(plugin, "frate_new_plugin"));
    if (createPod == nullptr)
    {
        output = "Built-in plugin is missing its frate_new_plugin entry point.";
        return true;
    }

    createPod(words[1].toRawUTF8(), words.size() == 4 ? words[3].toRawUTF8() : "suite");
    output = workspace.getLastStatus();
    return true;
}
