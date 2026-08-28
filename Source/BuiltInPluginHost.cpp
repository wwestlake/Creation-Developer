#include "BuiltInPluginHost.h"
#include "ConsolePanel.h"

#include <string>

namespace
{
PluginPodWorkspace* activeWorkspace = nullptr;
ConsolePanel* activeReplConsole = nullptr;
BuiltInPluginHost* activeReplHost = nullptr;

const char* copyResponse(const juce::String& response)
{
    static thread_local std::string value;
    value = response.toStdString();
    return value.c_str();
}

extern "C" int64_t suite_create_plugin_pod(const char* name, const char* target)
{
    return activeWorkspace != nullptr && name != nullptr
        && activeWorkspace->createPluginPod(name, target != nullptr ? target : "suite") ? 1 : 0;
}

extern "C" const char* suite_repl_evaluate(const char* source)
{
    return copyResponse(activeReplConsole != nullptr && source != nullptr
        ? activeReplConsole->evaluateForPlugin(source)
        : "(FRust REPL session is unavailable)");
}

extern "C" const char* suite_repl_reset()
{
    return copyResponse(activeReplConsole != nullptr
        ? activeReplConsole->resetForPlugin()
        : "(FRust REPL session is unavailable)");
}

extern "C" const char* suite_repl_save()
{
    return copyResponse(activeReplConsole != nullptr
        ? activeReplConsole->saveForPlugin()
        : "(FRust REPL session is unavailable)");
}

extern "C" const char* suite_repl_load()
{
    return copyResponse(activeReplConsole != nullptr
        ? activeReplConsole->loadForPlugin()
        : "(FRust REPL session is unavailable)");
}

extern "C" int64_t suite_repl_clear()
{
    if (activeReplConsole == nullptr) return 0;
    activeReplConsole->clearForPlugin();
    return 1;
}

extern "C" const char* suite_repl_help(const char* command)
{
    return copyResponse(activeReplHost != nullptr && command != nullptr
        ? activeReplHost->getReplHelp(command)
        : "(FRust REPL help is unavailable)");
}
}

BuiltInPluginHost::BuiltInPluginHost(PluginPodWorkspace& workspaceToUse)
    : workspace(workspaceToUse)
{
}

BuiltInPluginHost::~BuiltInPluginHost()
{
    if (activeReplHost == this)
        activeReplHost = nullptr;
    if (replPlugin != nullptr)
        frust_plugin_unload(replPlugin);
    if (podPlugin != nullptr)
        frust_plugin_unload(podPlugin);
}

void BuiltInPluginHost::attachReplConsole(ConsolePanel& console)
{
    replConsole = &console;
}

bool BuiltInPluginHost::load(juce::String& error)
{
    activeWorkspace = &workspace;
    activeReplConsole = replConsole;
    activeReplHost = this;
    frust_plugin_host_set_application_identity("creation-developer");
    frust_plugin_register_host_function("suite_create_plugin_pod", reinterpret_cast<void*>(&suite_create_plugin_pod));
    frust_plugin_register_host_function("suite_repl_evaluate", reinterpret_cast<void*>(&suite_repl_evaluate));
    frust_plugin_register_host_function("suite_repl_reset", reinterpret_cast<void*>(&suite_repl_reset));
    frust_plugin_register_host_function("suite_repl_save", reinterpret_cast<void*>(&suite_repl_save));
    frust_plugin_register_host_function("suite_repl_load", reinterpret_cast<void*>(&suite_repl_load));
    frust_plugin_register_host_function("suite_repl_clear", reinterpret_cast<void*>(&suite_repl_clear));
    frust_plugin_register_host_function("suite_repl_help", reinterpret_cast<void*>(&suite_repl_help));

    const auto pluginsDirectory = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                                      .getParentDirectory().getChildFile("plugins");
    const auto podPluginFile = pluginsDirectory.getChildFile("SuitePluginPodCommands.frust");
    const auto replPluginFile = pluginsDirectory.getChildFile("FrustReplPlugin.frust");
    if (! podPluginFile.existsAsFile() || ! replPluginFile.existsAsFile())
    {
        error = "Built-in plugin source is missing from " + pluginsDirectory.getFullPathName();
        return false;
    }

    podPlugin = frust_plugin_load(podPluginFile.getFullPathName().toRawUTF8());
    if (podPlugin == nullptr)
    {
        error = juce::String(frust_plugin_last_error());
        return false;
    }

    replPlugin = frust_plugin_load(replPluginFile.getFullPathName().toRawUTF8());
    if (replPlugin == nullptr)
    {
        error = juce::String(frust_plugin_last_error());
        return false;
    }

    registerReplHelpPlugin("repl", "FRust Interactive commands", replPlugin);
    frust_plugin_call_on_init(podPlugin);
    frust_plugin_call_on_init(replPlugin);
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

    if (podPlugin == nullptr)
    {
        output = "The Suite Plugin Pod Commands plugin is not loaded.";
        return true;
    }

    auto createPod = reinterpret_cast<int64_t (*)(const char*, const char*)>(frust_plugin_get_fn(podPlugin, "frate_new_plugin"));
    if (createPod == nullptr)
    {
        output = "Built-in plugin is missing its frate_new_plugin entry point.";
        return true;
    }

    createPod(words[1].toRawUTF8(), words.size() == 4 ? words[3].toRawUTF8() : "suite");
    output = workspace.getLastStatus();
    return true;
}

void BuiltInPluginHost::registerReplHelpPlugin(const juce::String& name,
                                               const juce::String& summary,
                                               FrustPluginHandle plugin)
{
    if (name.trim().isEmpty() || plugin == nullptr)
        return;

    for (auto& provider : replHelpProviders)
    {
        if (provider.name.equalsIgnoreCase(name))
        {
            provider.summary = summary;
            provider.plugin = plugin;
            return;
        }
    }

    replHelpProviders.push_back({ name.trim(), summary, plugin });
}

bool BuiltInPluginHost::processReplCommand(const juce::String& input, juce::String& output)
{
    if (replPlugin == nullptr)
    {
        output = "The FRust REPL plugin is not loaded.";
        return true;
    }

    auto submit = reinterpret_cast<const char* (*)(const char*)>(frust_plugin_get_fn(replPlugin, "frust_repl_submit"));
    if (submit == nullptr)
    {
        output = "Built-in REPL plugin is missing its frust_repl_submit entry point.";
        return true;
    }

    output = juce::String(submit(input.toRawUTF8()));
    return true;
}

juce::String BuiltInPluginHost::getReplBanner() const
{
    if (replPlugin == nullptr) return {};
    auto banner = reinterpret_cast<const char* (*)()>(frust_plugin_get_fn(replPlugin, "frust_repl_banner"));
    return banner != nullptr ? juce::String(banner()) : juce::String();
}

juce::String BuiltInPluginHost::getReplPrompt() const
{
    if (replPlugin == nullptr) return {};
    auto prompt = reinterpret_cast<const char* (*)()>(frust_plugin_get_fn(replPlugin, "frust_repl_prompt"));
    return prompt != nullptr ? juce::String(prompt()) : juce::String();
}

juce::String BuiltInPluginHost::getReplHelp(const juce::String& command) const
{
    const auto topic = command.substring(5).trim();
    if (topic.isEmpty())
    {
        juce::String result = "FRust Interactive commands:\n  @help [plugin]  Show general or plugin-specific help\n  @clear          Clear the terminal transcript\n  @reset          Forget the current FRust bindings\n  @save           Save the current session\n  @load           Restore the saved session\n\nREPL help providers:";
        for (const auto& provider : replHelpProviders)
            result << "\n  " << provider.name << " - " << provider.summary;
        return result;
    }

    for (const auto& provider : replHelpProviders)
    {
        if (! provider.name.equalsIgnoreCase(topic))
            continue;

        auto help = reinterpret_cast<const char* (*)()>(frust_plugin_get_fn(provider.plugin, "frust_repl_help"));
        return help != nullptr
            ? juce::String(help())
            : "The '" + provider.name + "' REPL plugin does not provide frust_repl_help().";
    }

    return "No REPL help provider named '" + topic + "'. Type @help to list available providers.";
}
