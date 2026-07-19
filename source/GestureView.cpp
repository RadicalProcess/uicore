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
    , soundfileLabel_("soundfileLabel", "")
    , selectButton_("Select soundfile...")
    , dropHintLabel_("dropHintLabel", "or drag & drop")
    , usageLabel_("usageLabel", "used 0 times")
    {
        // Needed to read soundfile headers when validating a dropped file.
        formatManager_.registerBasicFormats();

        // Populate the dropdown with every key; ComboBox item ids must be >= 1,
        // so offset the MIDI note by one.
        for (auto note = firstKey; note <= lastKey; ++note)
            keyBox_.addItem(keyName(note), note - firstKey + 1);

        keyBox_.setSelectedId(1, juce::dontSendNotification);

        // Grey out notes taken by other gestures each time the dropdown opens.
        keyBox_.onBeforePopup = [this] { refreshKeyAvailability(); };

        // Report a user pick to the host as a MIDI note. setKey never triggers
        // this: it selects with dontSendNotification.
        keyBox_.onChange = [this]
        {
            const auto id = keyBox_.getSelectedId();
            if (id == 0)
                return;

            if (onKeyChanged)
                onKeyChanged(id - 1 + firstKey);
        };
        addAndMakeVisible(keyBox_);

        // The gesture name is the only editable field.
        nameLabel_.setEditable(true);
        nameLabel_.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
        nameLabel_.setJustificationType(juce::Justification::left);

        // Report a finished name edit to the host. setName never triggers this:
        // it sets the text with dontSendNotification.
        nameLabel_.onTextChange = [this]
        {
            if (onNameChanged)
                onNameChanged(nameLabel_.getText());
        };
        addAndMakeVisible(nameLabel_);
        soundfileLabel_.setJustificationType(juce::Justification::left);
        addAndMakeVisible(soundfileLabel_);

        selectButton_.onClick = [this] { openFileChooser(); };
        addAndMakeVisible(selectButton_);

        dropHintLabel_.setJustificationType(juce::Justification::left);
        addAndMakeVisible(dropHintLabel_);

        usageLabel_.setJustificationType(juce::Justification::left);
        addAndMakeVisible(usageLabel_);

        addAndMakeVisible(thumbnail_);

        // Child controls swallow mouse clicks before they reach this view, so a
        // click on a label, the key box or a button would otherwise never select
        // the row. Listen to every child (nested editors included) so a click
        // anywhere in the row still forwards to mouseDown and selects it.
        for (auto* child : getChildren())
            child->addMouseListener(this, true);

        // Rows start without a soundfile: show the select button and drop hint.
        updateSoundfileArea();
    }

    void GestureView::paint(juce::Graphics &g)
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colour(juce::Colours::white));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 5.0f, 1.0f);

        if (selected_)
        {
            g.setColour(juce::Colours::cornflowerblue);
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.5f), 5.0f, 2.0f);
        }

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

    void GestureView::mouseDown(const juce::MouseEvent &)
    {
        if (onClicked)
            onClicked();
    }

    void GestureView::setName(const juce::String &name)
    {
        nameLabel_.setText(name, juce::dontSendNotification);
    }

    void GestureView::setKey(int midiNote)
    {
        // The dropdown item id is the MIDI note offset by one (see the constructor).
        keyBox_.setSelectedId(midiNote - firstKey + 1, juce::dontSendNotification);
    }

    void GestureView::setTrajectory(const std::vector<TrajectoryView::Anchor> &anchors)
    {
        thumbnail_.setAnchorData(anchors);
    }

    void GestureView::setSoundfile(const juce::String &name)
    {
        soundfileLabel_.setText(name, juce::dontSendNotification);
        updateSoundfileArea();
    }

    void GestureView::updateSoundfileArea()
    {
        // "used n times" is meaningless without a soundfile, so it swaps out
        // together with the name label.
        const auto associated = soundfileLabel_.getText().isNotEmpty();

        soundfileLabel_.setVisible(associated);
        usageLabel_.setVisible(associated);
        selectButton_.setVisible(!associated);
        dropHintLabel_.setVisible(!associated);
    }

    void GestureView::openFileChooser()
    {
        fileChooser_ = std::make_unique<juce::FileChooser>(
            "Select a soundfile", juce::File(), "*.wav;*.aiff;*.aif");

        const auto flags = juce::FileBrowserComponent::openMode
                         | juce::FileBrowserComponent::canSelectFiles;

        fileChooser_->launchAsync(flags, [this](const juce::FileChooser &chooser)
        {
            const auto file = chooser.getResult();

            // An invalid file object means the dialog was cancelled.
            if (file == juce::File())
                return;

            // The native browser can only filter by extension, so a picked file
            // can still be rejected (e.g. multichannel); tell the user why.
            if (!isAcceptedSoundfile(file.getFullPathName()))
            {
                juce::NativeMessageBox::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon, "Unsupported soundfile",
                    "Only a single mono .wav / .aiff soundfile can be used.");
                return;
            }

            setSoundfile(file.getFileName());

            if (onSoundfileDropped)
                onSoundfileDropped(file);
        });
    }

    void GestureView::setSelected(bool selected)
    {
        if (selected_ == selected)
            return;

        selected_ = selected;
        repaint();
    }

    void GestureView::refreshKeyAvailability()
    {
        for (auto note = firstKey; note <= lastKey; ++note)
            keyBox_.setItemEnabled(note - firstKey + 1, true);

        if (!unavailableKeysProvider)
            return;

        for (const auto note : unavailableKeysProvider())
            keyBox_.setItemEnabled(note - firstKey + 1, false);
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
        setSoundfile(file.getFileName());

        if (onSoundfileDropped)
            onSoundfileDropped(file);
    }

    void GestureView::resized() {
        keyBox_.setBounds(5, 5, 70, 30);
        nameLabel_.setBounds(90, 7, 100, 26);
        soundfileLabel_.setBounds(5, 50, 140, 20);
        usageLabel_.setBounds(160, 50, 14, 20);

        // The select button and drop hint occupy the soundfile line, which is
        // free while no soundfile is associated (label and usage are hidden).
        selectButton_.setBounds(5, 50, 100, 20);
        dropHintLabel_.setBounds(110, 50, 105, 20);

        thumbnail_.setBounds(220, 10, 60, 60);
    }
}
