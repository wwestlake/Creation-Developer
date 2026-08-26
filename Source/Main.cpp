#include <JuceHeader.h>

#include "MainComponent.h"

class CreationDeveloperApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Creation Developer"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>();
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        MainWindow()
            : juce::DocumentWindow("Creation Developer",
                                   juce::Colour(0xff111817),
                                   juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            centreWithSize(1380, 860);
            setResizable(true, true);
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(CreationDeveloperApplication)

