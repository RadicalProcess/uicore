add_library(UICore
        source/Font.cpp
        source/ComboBox.cpp
        source/StepSlider.cpp
        source/Keyboard.cpp
        source/KeyboardView.cpp
        source/Waveform.cpp
        source/WaveformRenderer.cpp
        source/GestureView.cpp
        source/ColorPalette.cpp
        source/TrajectoryThumbnail.cpp
        source/OpenGLView.cpp
        source/ObjModel.cpp
        source/ImageToggleButton.cpp
        source/IconButton.cpp
        source/GlissonSlider/HorizontalKnob.cpp
        source/GlissonSlider/VerticalKnob.cpp
        source/GlissonSlider/Slider.cpp
        source/GlissonSlider/DisabledRenderer.cpp
        source/GlissonSlider/EnabledRenderer.cpp
        source/GlissonSlider/Knob.cpp
        source/Label.cpp
        source/ProgressBar.cpp
        source/LevelIndicator.cpp)


add_library(rp::UICore ALIAS UICore)

target_include_directories(UICore
        PRIVATE include/UICore
        PUBLIC include)

juce_add_binary_data(UIResource
        SOURCES
        resource/RobotoCondensed.ttf
        resource/Logo.png
        resource/dummyhead.obj
        resource/icons/play.svg
        resource/icons/pause.svg
        resource/icons/stop.svg
        resource/icons/room.svg
        resource/icons/settings.svg
        resource/icons/trash.svg
        resource/icons/pencil.svg
        resource/icons/eye.svg
        resource/icons/plus.svg
        resource/icons/minus.svg
        resource/icons/chart-line.svg
        resource/icons/scissors.svg
        resource/icons/wave.svg
        resource/icons/width.svg)

target_link_libraries(UICore
        PUBLIC
        UIResource
        juce::juce_audio_formats
        juce::juce_audio_utils
        juce::juce_gui_basics
        juce::juce_opengl)

target_compile_features(UICore PUBLIC cxx_std_17)
