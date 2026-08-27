#include "FrateTerminalPanel.h"

namespace
{
juce::String normalizePath(juce::String path)
{
    path = path.replaceCharacter('\\', '/').trim();
    while (path.startsWithChar('/')) path = path.substring(1);
    while (path.endsWithChar('/')) path = path.dropLastCharacters(1);
    return path;
}
}

FrateTerminalPanel::FrateTerminalPanel()
{
    transcript.setMultiLine(true, true);
    transcript.setReadOnly(true);
    transcript.setScrollbarsShown(true);
    transcript.setFont(juce::Font("Consolas", 13.0f, juce::Font::plain));
    transcript.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff141414));
    transcript.setColour(juce::TextEditor::textColourId, juce::Colour(0xffd7ffe4));
    transcript.setText("Frate VFS Terminal\nType 'help' for commands.\n", juce::dontSendNotification);
    addAndMakeVisible(transcript);

    prompt.setColour(juce::Label::textColourId, juce::Colour(0xff5ee6a8));
    prompt.setFont(juce::Font("Consolas", 13.0f, juce::Font::bold));
    addAndMakeVisible(prompt);

    input.setFont(juce::Font("Consolas", 13.0f, juce::Font::plain));
    input.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff202020));
    input.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    input.onReturnKey = [this] { submit(); };
    addAndMakeVisible(input);

    clearButton.onClick = [this] { transcript.clear(); };
    addAndMakeVisible(clearButton);
}

void FrateTerminalPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
    g.setColour(juce::Colour(0xff333333));
    g.drawRect(getLocalBounds(), 1);
}

void FrateTerminalPanel::resized()
{
    auto area = getLocalBounds().reduced(6);
    auto inputRow = area.removeFromBottom(28);
    clearButton.setBounds(inputRow.removeFromRight(54));
    inputRow.removeFromRight(6);
    prompt.setBounds(inputRow.removeFromLeft(58));
    input.setBounds(inputRow);
    area.removeFromBottom(6);
    transcript.setBounds(area);
}

void FrateTerminalPanel::appendOutput(const juce::String& text)
{
    transcript.moveCaretToEnd();
    transcript.insertTextAtCaret(text + "\n");
}

juce::String FrateTerminalPanel::resolveDirectory(const juce::String& requestedPath) const
{
    auto requested = normalizePath(requestedPath);
    if (requested.isEmpty() || requested == ".") return currentDirectory;
    if (requested == "..")
    {
        const auto lastSlash = currentDirectory.lastIndexOfChar('/');
        return lastSlash > 0 ? currentDirectory.substring(0, lastSlash) : currentDirectory;
    }
    if (requested.startsWith("Assets/")) return requested;
    return currentDirectory + "/" + requested;
}

void FrateTerminalPanel::listDirectory()
{
    if (! listProjectEntries)
    {
        appendOutput("No Suite project is open.");
        return;
    }

    const auto prefix = currentDirectory + "/";
    juce::StringArray children;
    for (const auto& path : listProjectEntries())
    {
        if (! path.startsWith(prefix)) continue;
        const auto remainder = path.substring(prefix.length());
        const auto slash = remainder.indexOfChar('/');
        children.addIfNotAlreadyThere(slash >= 0 ? remainder.substring(0, slash) + "/" : remainder);
    }
    children.sort(true);
    appendOutput(children.isEmpty() ? "(empty)" : children.joinIntoString("\n"));
}

void FrateTerminalPanel::submit()
{
    const auto command = input.getText().trim();
    input.clear();
    if (command.isEmpty()) return;
    appendOutput("frate> " + command);

    auto normalizedCommand = command;
    if (normalizedCommand.startsWithIgnoreCase("frate "))
        normalizedCommand = normalizedCommand.substring(6).trimStart();

    const auto words = juce::StringArray::fromTokens(normalizedCommand, " \t\r\n", "");
    if (words.isEmpty()) return;
    const auto verb = words[0].toLowerCase();
    if (verb == "help")
    {
        appendOutput("Commands: help, pwd, ls, cd <pod-or-path>, new <pod> [--for <target>], clear");
    }
    else if (verb == "pwd")
    {
        appendOutput("/" + currentDirectory);
    }
    else if (verb == "ls")
    {
        listDirectory();
    }
    else if (verb == "cd")
    {
        if (words.size() != 2) appendOutput("Usage: cd <pod-or-path>");
        else { currentDirectory = resolveDirectory(words[1]); appendOutput("/" + currentDirectory); }
    }
    else if (verb == "clear")
    {
        transcript.clear();
    }
    else if (processFrateAction)
    {
        juce::String output;
        if (processFrateAction(normalizedCommand, output)) appendOutput(output);
        else appendOutput("Unknown command. Type 'help' for commands.");
    }
    else
    {
        appendOutput("Frate commands are unavailable.");
    }
}
