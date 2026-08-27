#pragma once

#include <JuceHeader.h>

class FrateTerminalPanel final : public juce::Component
{
public:
    FrateTerminalPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<bool(const juce::String&, juce::String&)> processFrateAction;
    std::function<juce::StringArray()> listProjectEntries;

private:
    void submit();
    void appendOutput(const juce::String& text);
    juce::String resolveDirectory(const juce::String& requestedPath) const;
    void listDirectory();

    juce::TextEditor transcript;
    juce::Label prompt { "Prompt", "frate>" };
    juce::TextEditor input;
    juce::TextButton clearButton { "Clear" };
    juce::String currentDirectory { "Assets/Source/FRust/PluginPods" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrateTerminalPanel)
};
