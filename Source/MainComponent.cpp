#include "MainComponent.h"

#include "Branding.h"
#include "ConsolePanel.h"
#include "FrateTerminalPanel.h"
#include "TerminalPanel.h"

#include <creation/ui/SuiteAiChatPanel.h>
#include <creation/services/SuiteVfsJsonStore.h>

MainComponent::MainComponent()
{
    configureHeader();

    dockManager = std::make_unique<CreationDock::DockManager>(*this);
    addAndMakeVisible(*dockManager);

    dockManager->registerPanel(
        "frust-workspace",
        "FRust Workspace",
        makeSummaryPanel("The standalone FRust IDE remains the active editor during migration.\n\n"
                         "Next: embed its project/editor services through an app-owned adapter, then expose FRust pod and tool actions inside this shell."),
        CreationDock::DockTargetZone::CenterTab);

    dockManager->registerPanel(
        "suite-plugin-host",
        "Suite Plugin Host",
        makeSummaryPanel("Creation Developer is a first-order Suite plugin host.\n\n"
                         "Next: consume the shared, versioned plugin-host SDK and expose extension points for panels, commands, templates, generators, formatters, and language tools."),
        CreationDock::DockTargetZone::Left);

    auto virtualEngineer = std::make_unique<creation::ui::SuiteAiChatPanel>();
    virtualEngineer->RefreshConfiguredAccounts();
    dockManager->registerPanel("virtual-engineer", "Virtual Engineer", std::move(virtualEngineer),
                               CreationDock::DockTargetZone::Right);

    auto communicationsPanel = makeSummaryPanel();
    communicationsSummary = communicationsPanel.get();
    dockManager->registerPanel("suite-communications", "Suite Communications", std::move(communicationsPanel),
                               CreationDock::DockTargetZone::Bottom);

    auto terminal = std::make_unique<TerminalPanel>();
    terminal->getProjectRoot = [] { return juce::File::getCurrentWorkingDirectory(); };
    dockManager->registerPanel("os-terminal", "OS Terminal", std::move(terminal),
                               CreationDock::DockTargetZone::Bottom);

    auto replConsole = std::make_unique<ConsolePanel>();
    replConsole->getProjectRoot = [] { return juce::File::getCurrentWorkingDirectory(); };
    dockManager->registerPanel("frust-repl", "FRust Terminal", std::move(replConsole),
                               CreationDock::DockTargetZone::Bottom);

    auto frateTerminal = std::make_unique<FrateTerminalPanel>();
    frateTerminal->processFrateAction = [this](const juce::String& input, juce::String& output)
    {
        return builtInPluginHost.processFrateCommand(input, output);
    };
    frateTerminal->listProjectEntries = [this] { return pluginPodWorkspace.getSession().listEntryPaths(); };
    dockManager->registerPanel("frate-terminal", "Frate Terminal", std::move(frateTerminal),
                               CreationDock::DockTargetZone::Bottom);

    juce::String pluginError;
    if (! builtInPluginHost.load(pluginError))
        headerBar.setStatusText("Built-in plugin commands unavailable: " + pluginError);

    refreshSuiteCommunications();
    restoreActiveProject();
    setSize(1380, 860);
}

MainComponent::~MainComponent() = default;

void MainComponent::configureHeader()
{
    headerBar.setAppTitle("Creation Developer");
    headerBar.setAppLogo(creation::ui::SuiteLogoId::suite);
    headerBar.setProjectLabel("Project: None");
    headerBar.setTransportControlsVisible(false);
    headerBar.audioButton.setButtonText("Refresh");
    headerBar.tourButton.setButtonText("EULA");
    headerBar.onAudioRequested = [this] { refreshSuiteCommunications(); };
    headerBar.onTourRequested = [this] { suiteShellController.showSuiteEula(); };
    suiteShellController.attach(headerBar,
                                { "Creation Developer", creation::assets::SuiteAppDomain::developer,
                                  creation_developer::branding::backgroundColour() },
                                [this](const juce::String& status) { headerBar.setStatusText(status); });
    suiteShellController.onProjectOpenRequested = [this](const juce::String& projectId)
    {
        openActiveProject(projectId);
    };
    addAndMakeVisible(headerBar);
}

void MainComponent::openActiveProject(const juce::String& projectId)
{
    const auto& projectSession = suiteShellController.getActiveProjectSession();
    if (! projectSession.isValid() || projectSession.getProjectId() != projectId)
    {
        headerBar.setProjectLabel("Project: None");
        headerBar.setStatusText("The shared Project Manager did not provide an active project session.");
        return;
    }

    pluginPodWorkspace.useProject(projectSession);
    headerBar.setProjectLabel("Project: " + pluginPodWorkspace.getSession().getManifest().projectName);
    headerBar.setStatusText("Opened project " + pluginPodWorkspace.getSession().getManifest().projectName + ".");
    auto* state = new juce::DynamicObject();
    state->setProperty("lastOpenedProjectId", projectId);
    juce::String saveError;
    creation::services::SuiteVfsJsonStore::saveJson("creation-developer-settings.json", juce::var(state), saveError);
}

void MainComponent::restoreActiveProject()
{
    juce::String loadError;
    const auto state = creation::services::SuiteVfsJsonStore::loadJson("creation-developer-settings.json", loadError);
    if (const auto* settings = state.getDynamicObject())
        if (const auto projectId = settings->getProperty("lastOpenedProjectId").toString(); projectId.isNotEmpty())
            suiteShellController.openProject(projectId);
}

void MainComponent::refreshSuiteCommunications()
{
    juce::String settingsError;
    const auto suiteSettings = suiteSettingsStore.load(settingsError);
    juce::String aiError;
    const auto aiSettings = suiteAiSettingsStore.load(aiError);
    const auto runtime = creation::services::SuiteAiSettingsResolver::resolveRuntimeSettingsForApp(
        aiSettings, creation::assets::SuiteAppDomain::developer);

    juce::String summary;
    summary << "Shared AI accounts: " << aiSettings.accounts.size() << "\n";
    summary << "Resolved provider: " << runtime.providerDisplayName << "\n";
    summary << "Resolved model: " << runtime.modelName << "\n";
    summary << "Suite VFS root: " << suiteSettings.suiteVfsRoot << "\n\n";
    summary << "Creation Developer uses the Suite's configured BYOK accounts and communications services. No separate provider or credential store is created here.";
    if (communicationsSummary != nullptr)
        communicationsSummary->setText(summary, juce::dontSendNotification);

    if (settingsError.isNotEmpty())
        headerBar.setStatusText("Suite settings: " + settingsError);
    else if (aiError.isNotEmpty())
        headerBar.setStatusText("AI settings: " + aiError);
    else
        headerBar.setStatusText("Shared Suite communications ready.");
}

std::unique_ptr<juce::TextEditor> MainComponent::makeSummaryPanel(const juce::String& text)
{
    auto editor = std::make_unique<juce::TextEditor>();
    editor->setMultiLine(true);
    editor->setReadOnly(true);
    editor->setCaretVisible(false);
    editor->setScrollbarsShown(true);
    editor->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff131d1b));
    editor->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff3c514b));
    editor->setColour(juce::TextEditor::textColourId, juce::Colour(0xffecf4ef));
    editor->setText(text, juce::dontSendNotification);
    return editor;
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
    auto contentArea = getLocalBounds();
    headerBar.setBounds(contentArea.removeFromTop(96));
    if (dockManager != nullptr)
        dockManager->setBounds(contentArea.reduced(18));
}
