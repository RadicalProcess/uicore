add_library(UICore
        source/Font.cpp
        source/ComboBox.cpp
        source/StepSlider.cpp
        source/KeyboardView.cpp
        source/Waveform.cpp
        source/WaveformRenderer.cpp
        source/MotionView.cpp
        source/GestureView.cpp
        source/ThumbnailComponent.cpp
        source/OpenGLView.cpp
        source/ObjModel.cpp
        source/ImageToggleButton.cpp
        source/GlissonSlider/HorizontalKnob.cpp
        source/GlissonSlider/VerticalKnob.cpp
        source/GlissonSlider/Slider.cpp
        source/GlissonSlider/DisabledRenderer.cpp
        source/GlissonSlider/EnabledRenderer.cpp
        source/GlissonSlider/Knob.cpp
        source/Label.cpp
        source/ProgressBar.cpp)


add_library(rp::UICore ALIAS UICore)

target_include_directories(UICore
        PRIVATE include/UICore
        PUBLIC include)

juce_add_binary_data(UIResource
        SOURCES
        resource/RobotoCondensed.ttf
        resource/Logo.png
        resource/head_lowpoly.obj)

target_link_libraries(UICore
        PUBLIC
        UIResource
        juce::juce_audio_formats
        juce::juce_audio_utils
        juce::juce_gui_basics
        juce::juce_opengl)

target_compile_features(UICore PUBLIC cxx_std_17)
