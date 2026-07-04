#include "GestureView.h"

namespace rp::uicore
{
    namespace
    {
        // Offer the full MIDI note range (0, C-1) to (127, G9) rather than just
        // the 88 keys of a piano keyboard.
        const auto firstKey = 0;
        const auto lastKey = 127;

        juce::String keyName(int midiNote)
        {
            static const char* const names[] =
                { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

            const auto octave = midiNote / 12 - 1;
            return juce::String(names[midiNote % 12]) + juce::String(octave);
        }
    }

    GestureView::GestureView()
    : keyBox_("keyBox")
    , nameLabel_("nameLabel", "gesture 1")
    , soundfileLabel_("soundfileLabel", "sound.wav")
    , usageLabel_("usageLabel", "used 0 times")
    {
        // Needed to read soundfile headers when validating a dropped file.
        formatManager_.registerBasicFormats();

        // Populate the dropdown with every key; ComboBox item ids must be >= 1,
        // so offset the MIDI note by one.
        for (auto note = firstKey; note <= lastKey; ++note)
            keyBox_.addItem(keyName(note), note - firstKey + 1);

        keyBox_.setSelectedId(1, juce::dontSendNotification);
        keyBox_.setBounds(5, 5, 70, 30);
        addAndMakeVisible(keyBox_);

        // The gesture name is the only editable field.
        nameLabel_.setBounds(90, 7, 200, 26);
        nameLabel_.setEditable(true);
        nameLabel_.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
        addAndMakeVisible(nameLabel_);

        soundfileLabel_.setBounds(5, 50, 140, 20);
        addAndMakeVisible(soundfileLabel_);

        usageLabel_.setBounds(160, 50, 14, 20);
        addAndMakeVisible(usageLabel_);

        thumbnail_.setBounds(290, 7, 60, 60);
        addAndMakeVisible(thumbnail_);
    }

    void GestureView::paint(juce::Graphics &g)
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colour(juce::Colours::white));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 5.0f, 1.0f);
        if (!fileDragActive_)
            return;

        // Highlight the whole view and tell the user it is ready to accept the
        // file currently being dragged over it.
        const auto bounds = getLocalBounds();

        g.setColour(juce::Colours::cornflowerblue.withAlpha(0.2f));
        g.fillRect(bounds);

        g.setColour(juce::Colours::cornflowerblue);
        g.drawRect(bounds, 3);

        g.setColour(juce::Colours::white);
        g.drawText("Drop a mono .wav / .aiff soundfile here",
                   bounds.reduced(6), juce::Justification::centredBottom, false);
    }

    bool GestureView::isAcceptedSoundfile(const juce::String &path)
    {
        const juce::File file(path);

        const auto extension = file.getFileExtension().toLowerCase();
        if (extension != ".wav" && extension != ".aiff" && extension != ".aif")
            return false;


        const std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager_.createReaderFor(file));

        return reader != nullptr && reader->numChannels == 1;
    }

    bool GestureView::isInterestedInFileDrag(const juce::StringArray &files)
    {
        // Only a single accepted soundfile may be dropped; returning false for
        // everything else means the OS never shows a drop cue for it.
        return files.size() == 1 && isAcceptedSoundfile(files[0]);
    }

    void GestureView::fileDragEnter(const juce::StringArray &, int, int)
    {
        fileDragActive_ = true;
        repaint();
    }

    void GestureView::fileDragExit(const juce::StringArray &)
    {
        fileDragActive_ = false;
        repaint();
    }

    void GestureView::filesDropped(const juce::StringArray &files, int, int)
    {
        fileDragActive_ = false;
        repaint();

        // isInterestedInFileDrag has already vetted the drop, but guard anyway.
        if (files.size() != 1 || !isAcceptedSoundfile(files[0]))
            return;

        const juce::File file(files[0]);

        // The label shows only the file name; the callback carries the full file
        // so the host can locate it (e.g. to notify with the full path).
        soundfileLabel_.setText(file.getFileName(), juce::dontSendNotification);

        if (onSoundfileDropped)
            onSoundfileDropped(file);
    }

}
