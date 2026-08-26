#pragma once

#include <JuceHeader.h>

#include <creation/services/SuiteAiSettings.h>
#include <creation/suite/SuiteSettings.h>
#include <creation/ui/CreationSuiteHeaderBar.h>
#include <creation/ui/SuiteShellController.h>

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
    static void configureSummary(juce::TextEditor& editor);

    CreationSuiteHeaderBar headerBar;
    creation::ui::SuiteShellController suiteShellController;
    creation::suite::SuiteSettingsStore suiteSettingsStore;
    creation::services::SuiteAiSettingsStore suiteAiSettingsStore;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::GroupComponent frustGroup;
    juce::GroupComponent pluginGroup;
    juce::GroupComponent communicationsGroup;
    juce::TextEditor frustSummary;
    juce::TextEditor pluginSummary;
    juce::TextEditor communicationsSummary;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

