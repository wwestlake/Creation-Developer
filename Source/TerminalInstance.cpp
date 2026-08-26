#include "TerminalInstance.h"

namespace { constexpr auto background = 0xff0d0d0d; constexpr auto text = 0xffd4d4d4; }

TerminalInstance::TerminalInstance(const juce::String& shellType, std::function<juce::File()> getRoot)
    : getProjectRoot(std::move(getRoot)), currentShell(shellType)
{
    currentWorkingDirectory = getProjectRoot ? getProjectRoot() : juce::File();
    if (!currentWorkingDirectory.isDirectory()) currentWorkingDirectory = juce::File::getCurrentWorkingDirectory();
    terminalText.setMultiLine(true, true); terminalText.setReturnKeyStartsNewLine(false); terminalText.setReadOnly(false);
    terminalText.setFont(juce::Font("Consolas", 13.0f, juce::Font::plain));
    terminalText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(background));
    terminalText.setColour(juce::TextEditor::textColourId, juce::Colour(text));
    terminalText.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    terminalText.onReturnKey = [this] { executeCommand(); };
    terminalText.addKeyListener(this); addAndMakeVisible(terminalText);
    juce::String error;
    if (session.start(currentShell, currentWorkingDirectory, error)) { appendOutput(juce::String(currentShell == "bash" ? "Bash" : "PowerShell") + " session started.\n"); startTimer(30); }
    else appendOutput("Terminal error: " + error + "\n");
    appendPrompt();
}

TerminalInstance::~TerminalInstance() { stopTimer(); terminalText.removeKeyListener(this); }
void TerminalInstance::paint(juce::Graphics& g) { g.fillAll(juce::Colour(background)); }
void TerminalInstance::resized() { terminalText.setBounds(getLocalBounds()); }
void TerminalInstance::setProjectRoot(const juce::File& root) { if (root.isDirectory()) currentWorkingDirectory = root; }
void TerminalInstance::appendPrompt() { const bool bash = currentShell == "bash"; const auto prompt = juce::String(bash ? "bash" : "PS") + " [" + currentWorkingDirectory.getFullPathName() + (bash ? "] $ " : "] > "); terminalText.moveCaretToEnd(); terminalText.setColour(juce::TextEditor::textColourId, bash ? juce::Colour(0xff4ec9b0) : juce::Colour(0xff569cd6)); terminalText.insertTextAtCaret(prompt); terminalText.setColour(juce::TextEditor::textColourId, juce::Colour(text)); inputStart = terminalText.getTotalNumChars(); terminalText.moveCaretToEnd(); terminalText.grabKeyboardFocus(); }
void TerminalInstance::appendOutput(const juce::String& output) { terminalText.moveCaretToEnd(); terminalText.insertTextAtCaret(output.replace("\r", "")); terminalText.moveCaretToEnd(); }
void TerminalInstance::updateWorkingDirectoryFromCommand(const juce::String& command) { auto trimmed = command.trim(); if (!trimmed.startsWithIgnoreCase("cd ") && !trimmed.startsWithIgnoreCase("set-location ")) return; auto target = trimmed.fromFirstOccurrenceOf(" ", false, false).trim().unquoted(); auto candidate = juce::File::isAbsolutePath(target) ? juce::File(target) : currentWorkingDirectory.getChildFile(target); if (candidate.isDirectory()) currentWorkingDirectory = candidate; }
void TerminalInstance::executeCommand() { const auto command = terminalText.getText().substring(inputStart).trimEnd(); if (command.isEmpty()) { appendOutput("\n"); appendPrompt(); return; } commandHistory.removeString(command); commandHistory.add(command); historyIndex = commandHistory.size(); juce::String error; if (!session.send(command + "\r\n", error)) { appendOutput("\nTerminal error: " + error + "\n"); appendPrompt(); return; } updateWorkingDirectoryFromCommand(command); appendOutput("\n"); awaitingCommandCompletion = currentShell == "powershell"; if (!awaitingCommandCompletion) appendPrompt(); }
void TerminalInstance::timerCallback() { const auto output = session.readAvailableOutput(); if (output.isNotEmpty()) { pendingShellOutput += output.replace("\r", ""); const auto marker = juce::String::charToString(30) + "FRUST_TERMINAL_DONE" + juce::String::charToString(30); while (pendingShellOutput.contains(marker)) { appendOutput(pendingShellOutput.upToFirstOccurrenceOf(marker, false, false)); pendingShellOutput = pendingShellOutput.fromFirstOccurrenceOf(marker, false, false); awaitingCommandCompletion = false; appendPrompt(); } if (!awaitingCommandCompletion && pendingShellOutput.isNotEmpty()) { appendOutput(pendingShellOutput); pendingShellOutput.clear(); } terminalText.moveCaretToEnd(); terminalText.grabKeyboardFocus(); } if (!session.isRunning()) { appendOutput("\nTerminal session ended.\n"); stopTimer(); } }
bool TerminalInstance::keyPressed(const juce::KeyPress& key, juce::Component*) { const auto caret = terminalText.getCaretPosition(); if ((key == juce::KeyPress::backspaceKey || key == juce::KeyPress::deleteKey) && caret <= inputStart) return true; if ((key.getTextCharacter() != 0 || key == juce::KeyPress::backspaceKey || key == juce::KeyPress::deleteKey) && caret < inputStart) { terminalText.moveCaretToEnd(); return false; } if (key == juce::KeyPress::upKey && !commandHistory.isEmpty()) { historyIndex = juce::jmax(0, historyIndex - 1); terminalText.setText(terminalText.getText().substring(0, inputStart) + commandHistory[historyIndex]); terminalText.moveCaretToEnd(); return true; } if (key == juce::KeyPress::downKey && !commandHistory.isEmpty()) { historyIndex = juce::jmin(commandHistory.size(), historyIndex + 1); terminalText.setText(terminalText.getText().substring(0, inputStart) + (historyIndex == commandHistory.size() ? juce::String() : commandHistory[historyIndex])); terminalText.moveCaretToEnd(); return true; } return false; }
