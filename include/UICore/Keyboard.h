#pragma once

#include <array>
#include <unordered_set>

#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{

    class Keyboard : public juce::Component
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;
            virtual void keyPressed(int keyNumber) = 0;
            virtual void keyReleased(int keyNumber) = 0;
        };

        Keyboard();
        ~Keyboard() override;

        void addListener(Listener* listener);
        void removeListener(Listener* listener);

        void setPressed(int keyNumber);
        void setReleased(int keyNumber);

    private:
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;

        struct KeyInfo
        {
            bool isBlackKey;
            juce::Rectangle<int> bounds;
            bool isPressed;
        };

        static constexpr int kNumKeys = 88;
        static constexpr int kFirstKey = 21;

        std::array<KeyInfo, kNumKeys> keys_;
        std::unordered_set<int> currentlyPressedKeys_;
        juce::ListenerList<Listener> listeners_;

        void calculateKeyBounds();
        int getKeyFromPosition(const juce::Point<int>& position) const;
        void notifyKeyPressed(int keyNumber);
        void notifyKeyReleased(int keyNumber);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Keyboard)
    };

}