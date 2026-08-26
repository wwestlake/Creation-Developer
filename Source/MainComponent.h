#pragma once

#include <JuceHeader.h>

#include <creation/services/SuiteAiSettings.h>
#include <creation/suite/SuiteSettings.h>
#include <creation/ui/CreationSuiteHeaderBar.h>
#include <creation/ui/SuiteShellController.h>
#include <CreationDock/DockManager.h>

class MainComponent final : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void configureHeader();
    void refreshSuiteCommunications();
    static std::unique_ptr<juce::TextEditor> makeSummaryPanel(const juce::String& text = {});

    CreationSuiteHeaderBar headerBar;
    creation::ui::SuiteShellController suiteShellController;
    creation::suite::SuiteSettingsStore suiteSettingsStore;
    creation::services::SuiteAiSettingsStore suiteAiSettingsStore;

    std::unique_ptr<CreationDock::DockManager> dockManager;
    juce::TextEditor* communicationsSummary = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
