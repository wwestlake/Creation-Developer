#pragma once

#include <creation/assets/ProjectSession.h>
#include <creation/suite/SuiteSettings.h>

class PluginPodWorkspace final
{
public:
    void useProject(const creation::assets::ProjectSession& projectSession);
    bool createPluginPod(const juce::String& podName, const juce::String& targetApplication);

    const creation::assets::ProjectSession& getSession() const noexcept { return session; }
    const juce::String& getLastStatus() const noexcept { return lastStatus; }

private:
    bool writeTextEntry(const juce::String& logicalPath, const juce::String& text);

    creation::suite::SuiteSettingsStore settingsStore;
    creation::suite::SuiteSettings settings;
    creation::assets::ProjectSession session;
    juce::String lastStatus;
};
