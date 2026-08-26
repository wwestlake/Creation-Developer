#include "MainComponent.h"

#include "Branding.h"

MainComponent::MainComponent()
{
    configureHeader();

    titleLabel.setText("Creation Developer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(32.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("FRust development and Creation Suite plugin authoring, built on the shared Suite platform.",
                          juce::dontSendNotification);
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc7d4cf));
    addAndMakeVisible(subtitleLabel);

    frustGroup.setText("FRust Workspace");
    pluginGroup.setText("Suite Plugin Host");
    communicationsGroup.setText("Suite Communications");
    addAndMakeVisible(frustGroup);
    addAndMakeVisible(pluginGroup);
    addAndMakeVisible(communicationsGroup);

    configureSummary(frustSummary);
    configureSummary(pluginSummary);
    configureSummary(communicationsSummary);
    addAndMakeVisible(frustSummary);
    addAndMakeVisible(pluginSummary);
    addAndMakeVisible(communicationsSummary);

    frustSummary.setText("The standalone FRust IDE remains the active editor during migration.\n\n"
                         "Next: embed its project/editor services through an app-owned adapter, then expose FRust pod and tool actions inside this shell.",
                         juce::dontSendNotification);
    pluginSummary.setText("Creation Developer will be a first-order Suite plugin host.\n\n"
                          "Next: consume the shared, versioned plugin-host SDK and expose extension points for panels, commands, templates, generators, formatters, and language tools.",
                          juce::dontSendNotification);

    refreshSuiteCommunications();
    setSize(1380, 860);
}

MainComponent::~MainComponent() = default;

void MainComponent::configureHeader()
{
    headerBar.setAppTitle("Creation Developer");
    headerBar.setAppLogo(creation::ui::SuiteLogoId::suite);
    headerBar.setProjectLabel("Developer shell: ready");
    headerBar.setTransportControlsVisible(false);
    headerBar.audioButton.setButtonText("Refresh");
    headerBar.tourButton.setButtonText("EULA");
    headerBar.onAudioRequested = [this] { refreshSuiteCommunications(); };
    headerBar.onTourRequested = [this] { suiteShellController.showSuiteEula(); };
    suiteShellController.attach(headerBar,
                                { "Creation Developer", creation::assets::SuiteAppDomain::unknown,
                                  creation_developer::branding::backgroundColour() },
                                [this](const juce::String& status) { headerBar.setStatusText(status); });
    addAndMakeVisible(headerBar);
}

void MainComponent::refreshSuiteCommunications()
{
    juce::String settingsError;
    const auto suiteSettings = suiteSettingsStore.load(settingsError);
    juce::String aiError;
    const auto aiSettings = suiteAiSettingsStore.load(aiError);
    const auto runtime = creation::services::SuiteAiSettingsResolver::resolveRuntimeSettingsForApp(
        aiSettings, creation::assets::SuiteAppDomain::unknown);

    juce::String summary;
    summary << "Shared AI accounts: " << aiSettings.accounts.size() << "\n";
    summary << "Resolved provider: " << runtime.providerDisplayName << "\n";
    summary << "Resolved model: " << runtime.modelName << "\n";
    summary << "Suite VFS root: " << suiteSettings.suiteVfsRoot << "\n\n";
    summary << "Creation Developer uses the Suite's configured BYOK accounts and communications services. No separate provider or credential store is created here.";
    communicationsSummary.setText(summary, juce::dontSendNotification);

    if (settingsError.isNotEmpty())
        headerBar.setStatusText("Suite settings: " + settingsError);
    else if (aiError.isNotEmpty())
        headerBar.setStatusText("AI settings: " + aiError);
    else
        headerBar.setStatusText("Shared Suite communications ready.");
}

void MainComponent::configureSummary(juce::TextEditor& editor)
{
    editor.setMultiLine(true);
    editor.setReadOnly(true);
    editor.setCaretVisible(false);
    editor.setScrollbarsShown(true);
    editor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff131d1b));
    editor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff3c514b));
    editor.setColour(juce::TextEditor::textColourId, juce::Colour(0xffecf4ef));
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(creation_developer::branding::backgroundColour());
    const auto bounds = getLocalBounds().toFloat().reduced(18.0f);
    g.setColour(creation_developer::branding::panelColour());
    g.fillRoundedRectangle(bounds, 24.0f);
    g.setColour(creation_developer::branding::accentColour().withAlpha(0.72f));
    g.drawRoundedRectangle(bounds, 24.0f, 1.2f);
}

void MainComponent::resized()
{
    headerBar.setBounds(getLocalBounds().removeFromTop(96));
    auto area = getLocalBounds().reduced(34, 28);
    area.removeFromTop(88);
    titleLabel.setBounds(area.removeFromTop(40));
    subtitleLabel.setBounds(area.removeFromTop(28));
    area.removeFromTop(18);

    auto top = area.removeFromTop(area.getHeight() / 2);
    auto left = top.removeFromLeft(top.getWidth() / 2);
    left.removeFromRight(8);
    top.removeFromLeft(8);
    frustGroup.setBounds(left);
    pluginGroup.setBounds(top);
    communicationsGroup.setBounds(area);

    frustSummary.setBounds(frustGroup.getBounds().reduced(14, 26));
    pluginSummary.setBounds(pluginGroup.getBounds().reduced(14, 26));
    communicationsSummary.setBounds(communicationsGroup.getBounds().reduced(14, 26));
}

