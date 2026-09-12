# UICore

A reusable library of custom [JUCE](https://juce.com/) GUI components and look-and-feels, built on top of `juce_gui_basics`. It provides a consistent visual style, a bundled font and logo, and a set of widgets (sliders, combo boxes, keyboards, waveform displays, etc.) for use across audio/GUI applications.

All code lives under the `rp::uicore` namespace and is exported as the CMake target **`rp::UICore`**.

## Components

| Component | Header | Description |
|-----------|--------|-------------|
| `ComboBox` | `UICore/ComboBox.h` | Styled drop-down selector |
| `Font` | `UICore/Font.h` | Access to the bundled Roboto Condensed font |
| `IconButton` | `UICore/IconButton.h` | SVG glyph on an outlined plate, plus the named buttons built on it (`PlayPauseButton`, `RoomButton`, `KeyButton`, …) |
| `ImageToggleButton` | `UICore/ImageToggleButton.h` | Toggle button rendered from images |
| `Keyboard` | `UICore/Keyboard.h` | On-screen piano keyboard with a `Listener` interface |
| `Label` | `UICore/Label.h` | Styled text label |
| `RotarySlider` | `UICore/RotarySlider.h` | Rotary slider look-and-feel |
| `StepSlider` | `UICore/StepSlider.h` | Discrete / stepped slider |
| `StraightSlider` | `UICore/StraightSlider.h` | Linear slider |
| `TextField` | `UICore/TextField.h` | Styled text editor with custom look-and-feel |
| `ToggleButton` | `UICore/ToggleButton.h` | Styled toggle button |
| `Waveform` | `UICore/Waveform.h` | Waveform display with playhead, optional click-and-drag region selection (resizable by its edges, movable by its body), and optional draggable fade-in/fade-out handles |
| `GlissonSlider` | `UICore/GlissonSlider/Slider.h` | Two-axis "glisson" slider (under `rp::uicore::glisson`) |
| `Style` | `UICore/Style.h` | Shared colors / styling constants (`rp::uicore::styles`) |
| `Utils` | `UICore/Utils.h` | Helper utilities |

## Dependencies

This library depends on **JUCE** (`juce_gui_basics`), but **JUCE is _not_ vendored or included as a submodule** in this repository.

The build assumes that JUCE's CMake targets and helper functions (`juce::juce_gui_basics`, `juce_add_binary_data`, `juce_add_gui_app`, …) are **already available in the surrounding CMake project**. In other words, UICore is meant to be consumed as a subdirectory of a parent project that brings its own copy of JUCE.

## Usage

Add UICore to a parent CMake project that already provides JUCE:

```cmake
# In the parent project, after JUCE is available
# (e.g. via add_subdirectory(JUCE) or FetchContent)
add_subdirectory(uicore)

target_link_libraries(MyApp PRIVATE rp::UICore)
```

Then include and use the components:

```cpp
#include <UICore/ComboBox.h>
#include <UICore/Keyboard.h>

rp::uicore::ComboBox box;
rp::uicore::Keyboard keyboard;
```

### Resources

Bundled assets (the Roboto Condensed font, a logo and the icon set) are compiled
into the `UIResource` binary-data target via `juce_add_binary_data` and linked
publicly into `rp::UICore`, so consumers automatically get them.

### Icons

The glyphs in `resource/icons/` are **[Lucide](https://lucide.dev)**, taken
unmodified from the set. Keep it that way: a hand-drawn or foreign glyph next to
them reads as a mistake, because the whole set shares one grid and one stroke.

Every file carries the upstream name in its `class` attribute
(`class="lucide lucide-box"`), which is what to search lucide.dev for — the
filename here is what the glyph *means* to us, and the two often differ
(`room.svg` is `box`, `width.svg` is `move-horizontal`, `question.svg` is
`circle-question-mark`).

`loadWhiteIcon` swaps `currentColor` for white and `IconButton` tints from
there, so download the icon as-is: 24x24 viewBox, `fill="none"`,
`stroke="currentColor"`, stroke width 2, round caps and joins.

Adding one is three edits: drop the `.svg` in `resource/icons/`, list it in the
`juce_add_binary_data(UIResource ...)` block, and add the `IconButton` subclass
naming it. The binary-data symbol strips punctuation from the filename, so
`key-round.svg` arrives as `BinaryData::keyround_svg`.

## Test application

**UITest**, a small JUCE GUI application that demonstrates and exercises the
components, lives in a separate repository:
[RadicalProcess/uitester](https://github.com/RadicalProcess/uitester). It
consumes this library as a submodule and brings its own copy of JUCE.

## Requirements

- C++17
- CMake 3.15+
- JUCE (provided by the parent project — not included here)

## License

GNU General Public License v3.0 — see [LICENSE](LICENSE).

The bundled Roboto Condensed font is licensed separately under the SIL Open Font
License; see [resource/OFL.txt](resource/OFL.txt).

The bundled icons are from [Lucide](https://lucide.dev), licensed separately
under the ISC license.
