#include "PluginPodWorkspace.h"
#include <creation/assets/ProjectWorkspaceService.h>

namespace
{
bool isValidPodName(const juce::String& name)
{
    return name == creation::suite::sanitizeProjectName(name) && name != "untitled-project";
}

juce::MemoryBlock asData(const juce::String& text)
{
    return { text.toRawUTF8(), static_cast<size_t>(text.getNumBytesAsUTF8()) };
}
}

bool PluginPodWorkspace::openProject(const juce::String& projectId)
{
    juce::String error;
    settings = settingsStore.load(error);
    if (! creation::assets::ProjectWorkspaceService::openProject(settings, projectId, session, error))
    {
        lastStatus = "Could not open the VFS project: " + error;
        return false;
    }

    lastStatus = "Opened VFS project " + session.getManifest().projectName + ".";
    return true;
}

bool PluginPodWorkspace::writeTextEntry(const juce::String& logicalPath, const juce::String& text)
{
    if (session.writeEntry(logicalPath, asData(text)))
        return true;

    lastStatus = "Could not write VFS entry " + logicalPath + ".";
    return false;
}

bool PluginPodWorkspace::createPluginPod(const juce::String& requestedName, const juce::String& requestedTarget)
{
    const auto podName = requestedName.trim().toLowerCase();
    const auto target = requestedTarget.trim().toLowerCase().isEmpty() ? juce::String("suite") : requestedTarget.trim().toLowerCase();
    const auto targetIdentity = target == "suite" ? juce::String("creation-suite") : "creation-" + target;
    if (! isValidPodName(podName))
    {
        lastStatus = "Plugin pod names use lowercase letters, numbers, '-' and '_'.";
        return false;
    }
    if (! juce::StringArray { "suite", "station", "engine", "movie", "live", "texture", "modeler", "developer" }.contains(target))
    {
        lastStatus = "Unknown plugin target '" + target + "'. Use suite, station, engine, movie, live, texture, modeler, or developer.";
        return false;
    }

    if (! session.isValid())
    {
        lastStatus = "Open a Suite project from the header before creating a plugin pod.";
        return false;
    }

    const auto root = juce::String("Assets/Source/FRust/PluginPods/") + podName + "/";
    const auto manifest = "{\n"
                          "  \"name\": \"" + podName + "\",\n"
                          "  \"version\": \"0.1.0\",\n"
                          "  \"type\": \"plugin\",\n"
                          "  \"description\": \"A Creation Suite FRust plugin.\"\n"
                          "}\n";
    const auto source = "manifest \"{\\\"name\\\":\\\"" + podName
                        + "\\\",\\\"version\\\":\\\"0.1.0\\\",\\\"intendedApplications\\\":[\\\"" + targetIdentity + "\\\"]}\";\n\n"
                          "pub fn on_init() -> i64 = {\n"
                          "    0\n"
                          "}\n";

    if (! writeTextEntry(root + "frate.json", manifest)
        || ! writeTextEntry(root + "src/main.fr", source)
        || ! writeTextEntry("Assets/Derived/FRust/PluginPods/" + podName + "/.keep", ""))
        return false;

    lastStatus = "Created " + target + " plugin pod '" + podName + "' in VFS project '" + session.getManifest().projectName + "'.";
    return true;
}
