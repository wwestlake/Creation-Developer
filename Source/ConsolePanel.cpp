#include "ConsolePanel.h"

ConsolePanel::ConsolePanel()
    : replSession(std::make_unique<frust::ReplSession>())
{
    headerLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    headerLabel.setColour(juce::Label::textColourId, juce::Colours::lightcyan);
    addAndMakeVisible(headerLabel);

    // One scrolling, editable view - a real console, not a separate output
    // pane + input box. Return submits the current line (via
    // setReturnKeyStartsNewLine(false), which routes it to onReturnKey
    // instead of inserting a newline) rather than starting a new one.
    consoleText.setMultiLine(true, true);
    consoleText.setReturnKeyStartsNewLine(false);
    consoleText.setReadOnly(false);
    consoleText.setCaretVisible(true);
    consoleText.setFont(juce::Font("Consolas", 13.0f, juce::Font::plain));
    consoleText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff141414));
    consoleText.setColour(juce::TextEditor::textColourId, juce::Colour(0xff00ff66));
    consoleText.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    consoleText.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    consoleText.setText(bannerText + promptText);
    consoleText.onReturnKey = [this] { evaluateInput(); };
    consoleText.addKeyListener(this);
    addAndMakeVisible(consoleText);

    clearButton.onClick = [this] { executeSubmittedInput(consoleText.getText(), "@clear"); };
    addAndMakeVisible(clearButton);

    resetButton.onClick = [this] { executeSubmittedInput(consoleText.getText(), "@reset"); };
    addAndMakeVisible(resetButton);

    saveButton.onClick = [this] { executeSubmittedInput(consoleText.getText(), "@save"); };
    addAndMakeVisible(saveButton);

    loadButton.onClick = [this] { executeSubmittedInput(consoleText.getText(), "@load"); };
    addAndMakeVisible(loadButton);
}

ConsolePanel::~ConsolePanel()
{
    consoleText.removeKeyListener(this);
}

void ConsolePanel::visibilityChanged()
{
    if (isShowing())
        consoleText.grabKeyboardFocus();
}

void ConsolePanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
    g.setColour(juce::Colour(0xff333333));
    g.drawRect(getLocalBounds(), 1);
}

void ConsolePanel::resized()
{
    auto bounds = getLocalBounds().reduced(6);

    auto topBar = bounds.removeFromTop(24);
    loadButton.setBounds(topBar.removeFromRight(50));
    topBar.removeFromRight(4);
    saveButton.setBounds(topBar.removeFromRight(50));
    topBar.removeFromRight(4);
    resetButton.setBounds(topBar.removeFromRight(55));
    topBar.removeFromRight(4);
    clearButton.setBounds(topBar.removeFromRight(50));
    headerLabel.setBounds(topBar);

    bounds.removeFromTop(4);
    consoleText.setBounds(bounds);
}

void ConsolePanel::evaluateInput()
{
    // Not tracked via caret position - deliberately just takes everything
    // after the last prompt in the buffer, so it still does the right
    // thing even if the user clicked around before pressing Return.
    auto fullText = consoleText.getText();
    auto lastPromptIndex = fullText.lastIndexOfIgnoreCase(promptText);
    if (lastPromptIndex < 0) return;

    auto input = fullText.substring(lastPromptIndex + promptText.length()).trim();

    executeSubmittedInput(fullText, input);
}

void ConsolePanel::executeSubmittedInput(const juce::String& fullText, const juce::String& input)
{
    if (input.isEmpty()) return;

    juce::String output;
    if (! processReplCommand || ! processReplCommand(input, output))
        output = "The FRust REPL plugin is unavailable.";

    if (clearRequestedByPlugin)
    {
        clearRequestedByPlugin = false;
        clearConsole();
        return;
    }

    const auto response = output.isNotEmpty() ? "\n  => " + output : juce::String();
    consoleText.setText(fullText + response + "\n" + promptText);
    consoleText.moveCaretToEnd();
}

bool ConsolePanel::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (key == juce::KeyPress::returnKey)
    {
        evaluateInput();
        return true;
    }

    return false;
}

void ConsolePanel::runScript(const juce::String& source, const juce::String& label)
{
    if (source.trim().isEmpty()) return;

    logMessage("(running " + label + " in the console session)");

    auto results = replSession->runScript(source.toStdString());
    for (auto& result : results) logMessage("  => " + juce::String(result));

    if (onSessionChanged) onSessionChanged();
}

void ConsolePanel::resetSession()
{
    replSession->reset();
    logMessage("(session reset - all bound variables forgotten)");
    if (onSessionChanged) onSessionChanged();
}

void ConsolePanel::setPluginPresentation(const juce::String& banner, const juce::String& prompt)
{
    bannerText = banner.isNotEmpty() ? banner : bannerText;
    promptText = prompt.isNotEmpty() ? prompt : promptText;
    clearConsole();
}

juce::String ConsolePanel::evaluateForPlugin(const juce::String& source)
{
    if (source.trim().isEmpty()) return {};

    const auto result = replSession->evaluate(source.toStdString());
    if (onSessionChanged) onSessionChanged();
    return juce::String(result);
}

juce::String ConsolePanel::resetForPlugin()
{
    replSession->reset();
    if (onSessionChanged) onSessionChanged();
    return "(session reset - all bound variables forgotten)";
}

juce::String ConsolePanel::saveForPlugin()
{
    auto file = getSessionFile();
    file.getParentDirectory().createDirectory();
    return file.replaceWithText(juce::String(replSession->exportAsJson()))
        ? "(session saved to " + file.getFullPathName() + ")"
        : "(failed to save session to " + file.getFullPathName() + ")";
}

juce::String ConsolePanel::loadForPlugin()
{
    auto file = getSessionFile();
    if (! file.existsAsFile())
        return "(no saved session at " + file.getFullPathName() + ")";

    if (! replSession->importFromJson(file.loadFileAsString().toStdString()))
        return "(failed to parse session file " + file.getFullPathName() + ")";

    if (onSessionChanged) onSessionChanged();
    return "(session loaded from " + file.getFullPathName() + ")";
}

void ConsolePanel::clearForPlugin()
{
    clearRequestedByPlugin = true;
}

juce::File ConsolePanel::getSessionFile() const
{
    auto root = getProjectRoot ? getProjectRoot() : juce::File();
    if (!root.isDirectory())
        root = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);

    return root.getChildFile(".frust").getChildFile("repl_session.json");
}

void ConsolePanel::saveSessionToFile()
{
    logMessage(saveForPlugin());
}

void ConsolePanel::loadSessionFromFile()
{
    logMessage(loadForPlugin());
}

void ConsolePanel::logMessage(const juce::String& message)
{
    // Inserted before the live prompt line so it doesn't get swallowed into
    // whatever the user is mid-typing.
    auto fullText = consoleText.getText();
    auto lastPromptIndex = fullText.lastIndexOfIgnoreCase(promptText);
    auto insertAt = lastPromptIndex >= 0 ? lastPromptIndex : fullText.length();

    consoleText.setText(fullText.substring(0, insertAt) + message + "\n" + fullText.substring(insertAt));
    consoleText.moveCaretToEnd();
}

void ConsolePanel::clearConsole()
{
    consoleText.setText(bannerText + promptText);
    consoleText.moveCaretToEnd();
}


