#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "ComboBox.h"
#include "Label.h"
#include "TrajectoryThumbnail.h"

#include <functional>
#include <vector>

namespace rp::uicore
{
    // Shows the properties of a single gesture in one horizontal row: the key
    // that triggers it, an editable gesture name, the soundfile it plays, how
    // many times it is used, and a thumbnail preview.
    //
    // Accepts a single mono .wav/.aiff/.aif soundfile dropped onto it. While a
    // droppable file hovers the view highlights itself; anything the filter
    // rejects (wrong extension, multichannel, or several files) is ignored. On a
    // successful drop the soundfile label shows the file name and onSoundfile
    // Dropped is called with the full file so the host can react (e.g. notify).
    class GestureView
        : public juce::Component
        , public juce::FileDragAndDropTarget
    {
    public:
        GestureView();

        void paint(juce::Graphics &) override;
        void mouseDown(const juce::MouseEvent &event) override;

        bool isInterestedInFileDrag(const juce::StringArray &files) override;
        void fileDragEnter(const juce::StringArray &files, int x, int y) override;
        void fileDragExit(const juce::StringArray &files) override;
        void filesDropped(const juce::StringArray &files, int x, int y) override;

        // Populate the row from a gesture's data.
        void setName(const juce::String &name);
        void setKey(int midiNote);
        void setTrajectory(const std::vector<TrajectoryView::Anchor> &anchors);

        // Draw the row as selected / unselected.
        void setSelected(bool selected);

        // Called after a valid soundfile is dropped, with the dropped file. The
        // view has already updated its own label by this point.
        std::function<void(const juce::File &file)> onSoundfileDropped;

        // Called when the row is clicked (outside its child controls), so the host
        // can select it.
        std::function<void()> onClicked;

        // Called when the user finishes editing the gesture name, with the new
        // text.
        std::function<void(const juce::String &name)> onNameChanged;

        // Called when the user picks a different key in the dropdown, with the new
        // MIDI note.
        std::function<void(int midiNote)> onKeyChanged;

        // Queried just before the key drop-down opens; returns the MIDI notes that
        // are already taken by other gestures and should be greyed out.
        std::function<std::vector<int>()> unavailableKeysProvider;

    private:

        // Re-enable every note, then disable the ones the provider reports as taken.
        void refreshKeyAvailability();

        // True only for a single mono .wav/.aiff/.aif file; used both to filter
        // drops and to decide whether the file may be read at all.
        bool isAcceptedSoundfile(const juce::String &path);

        // Reads soundfile headers so multichannel files can be rejected on drop.
        juce::AudioFormatManager formatManager_;

        // Set while an acceptable file is being dragged over the view so paint()
        // can highlight it.
        bool fileDragActive_ = false;

        // Whether this row is the selected one; drives the selection highlight.
        bool selected_ = false;

        // Selects the key that triggers the gesture; populated with every MIDI
        // note.
        ComboBox keyBox_;

        // Editable name of the gesture.
        Label nameLabel_;

        // Soundfile the gesture plays; not editable, set by dropping a file.
        Label soundfileLabel_;

        // How many times the gesture is used; not editable.
        Label usageLabel_;

        // Miniature preview of the gesture's trajectory curve.
        TrajectoryThumbnail thumbnail_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GestureView)
    };

}
