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
    //
    // While no soundfile is associated (setSoundfile with an empty name) the
    // soundfile area shows a button that opens the OS-native file browser plus a
    // hint that a file can also be dragged onto the row; a file picked in the
    // browser goes through the same validation and onSoundfileDropped path as a
    // dropped one.
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
        void setTrajectory(const std::vector<TrajectoryThumbnail::Anchor> &anchors);

        // Show the associated soundfile's name, or pass an empty name to show
        // the select-soundfile button and drag-and-drop hint instead.
        void setSoundfile(const juce::String &name);

        // Draw the row as selected / unselected.
        void setSelected(bool selected);

        // Set the colour of the left-edge swatch. onColorChanged is not fired.
        void setColor(juce::Colour colour);

        // Show a short status word next to the gesture name, drawn in the given
        // colour. An empty text hides the badge.
        void setStatusBadge(const juce::String &text, juce::Colour colour);

        // Called after a valid soundfile is dropped, with the dropped file. The
        // view has already updated its own label by this point.
        std::function<void(const juce::File &file)> onSoundfileDropped;

        // Called when the row is clicked anywhere, including on its child controls
        // (labels, key box, buttons), so the host can select it.
        std::function<void()> onClicked;

        // Called when the user finishes editing the gesture name, with the new
        // text.
        std::function<void(const juce::String &name)> onNameChanged;

        // Called when the user picks a colour from the swatch's palette popup.
        std::function<void(juce::Colour colour)> onColorChanged;

        // Called when the user picks a different key in the dropdown, with the new
        // MIDI note.
        std::function<void(int midiNote)> onKeyChanged;

        // Queried just before the key drop-down opens; returns the MIDI notes that
        // are already taken by other gestures and should be greyed out.
        std::function<std::vector<int>()> unavailableKeysProvider;

    private:

        // Re-enable every note, then disable the ones the provider reports as taken.
        void refreshKeyAvailability();

        // The left-edge colour swatch's bounds within this row.
        juce::Rectangle<int> colorStripBounds() const;

        // The status badge's bounds: the band between the name label and the
        // trajectory thumbnail.
        juce::Rectangle<int> statusBadgeBounds() const;

        // Open the colour palette popup anchored on the swatch.
        void openColorPalette();

        // Swap the soundfile area between the name label (soundfile associated)
        // and the select button plus drag-and-drop hint (no soundfile yet).
        void updateSoundfileArea();

        // Open the OS-native file browser; a valid pick is reported through
        // onSoundfileDropped, an invalid one shows a native alert.
        void openFileChooser();

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

        // Colour drawn in the left-edge swatch; picked from the palette popup.
        juce::Colour currentColour_;

        // Status badge text and colour; an empty text draws no badge.
        juce::String statusBadgeText_;
        juce::Colour statusBadgeColour_;

        // Selects the key that triggers the gesture; populated with every MIDI
        // note.
        ComboBox keyBox_;

        // Editable name of the gesture.
        Label nameLabel_;

        // Soundfile the gesture plays; not editable, set by dropping a file or
        // picking one in the file browser. Hidden while no soundfile is
        // associated.
        Label soundfileLabel_;

        // Opens the OS-native file browser; shown only while no soundfile is
        // associated.
        juce::TextButton selectButton_;

        // Tells the user a soundfile can also be dragged onto the row; shown
        // only while no soundfile is associated.
        Label dropHintLabel_;

        // Kept alive for the duration of the asynchronous native file dialog.
        std::unique_ptr<juce::FileChooser> fileChooser_;

        // How many times the gesture is used; not editable.
        Label usageLabel_;

        // Miniature preview of the gesture's trajectory curve.
        TrajectoryThumbnail thumbnail_;

        void resized() override;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GestureView)
    };

}
