#include <UICore/Keyboard.h>

namespace rp::uicore
{

    namespace
    {
        bool isBlackKey(int keyNumber)
        {
            const auto noteFromA = (keyNumber - 21) % 12;
            return noteFromA == 1 || noteFromA == 4 || noteFromA == 6 || noteFromA == 9 || noteFromA == 11;
        }
    }


    Keyboard::Keyboard()
    {
        for (size_t i = 0; i < kNumKeys; ++i)
        {
            const auto keyNumber = kFirstKey + static_cast<int>(i);
            keys_[i].isBlackKey = isBlackKey(keyNumber);
            keys_[i].isPressed = false;
        }
    }

    Keyboard::~Keyboard() = default;

    void Keyboard::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colours::darkgrey);

        for (size_t i = 0; i < kNumKeys; ++i)
        {
            const auto& key = keys_[i];

            if (!key.isBlackKey)
            {
                if (key.isPressed)
                    g.setColour(juce::Colours::yellow);
                else
                    g.setColour(juce::Colours::white);

                g.fillRect(key.bounds);
                g.setColour(juce::Colours::black);
                g.drawRect(key.bounds);
            }
        }

        for (size_t i = 0; i < kNumKeys; ++i)
        {
            const auto& key = keys_[i];

            if (key.isBlackKey)
            {
                if (key.isPressed)
                    g.setColour(juce::Colours::yellow);
                else
                    g.setColour(juce::Colours::black);

                g.fillRect(key.bounds);
                g.setColour(juce::Colours::darkgrey);
                g.drawRect(key.bounds);
            }
        }
    }

    void Keyboard::resized()
    {
        calculateKeyBounds();
    }

    void Keyboard::mouseDown(const juce::MouseEvent& event)
    {
        const auto keyNumber = getKeyFromPosition(event.position.toInt());
        if (keyNumber >= 0)
        {
            setPressed(keyNumber);
            notifyKeyPressed(keyNumber);
            currentlyPressedKeys_.insert(keyNumber);
        }
    }

    void Keyboard::mouseUp(const juce::MouseEvent&)
    {
        for (const auto keyNumber: currentlyPressedKeys_)
        {
            setReleased(keyNumber);
            notifyKeyReleased(keyNumber);
        }
        currentlyPressedKeys_.clear();
    }

    void Keyboard::mouseDrag(const juce::MouseEvent& event)
    {
        const auto keyNumber = getKeyFromPosition(event.position.toInt());
        if (keyNumber >= 0 && currentlyPressedKeys_.find(keyNumber) == currentlyPressedKeys_.end())
        {
            for (const auto oldKey: currentlyPressedKeys_)
            {
                setReleased(oldKey);
                notifyKeyReleased(oldKey);
            }
            currentlyPressedKeys_.clear();

            setPressed(keyNumber);
            notifyKeyPressed(keyNumber);
            currentlyPressedKeys_.insert(keyNumber);
        }
    }

    void Keyboard::addListener(Listener* listener)
    {
        listeners_.add(listener);
    }

    void Keyboard::removeListener(Listener* listener)
    {
        listeners_.remove(listener);
    }

    void Keyboard::setPressed(int keyNumber)
    {
        const auto index = keyNumber - kFirstKey;
        if (index >= 0 && index < kNumKeys)
        {
            const auto i = static_cast<size_t>(index);
            keys_[i].isPressed = true;
            repaint(keys_[i].bounds);
        }
    }

    void Keyboard::setReleased(int keyNumber)
    {
        const auto index = keyNumber - kFirstKey;
        if (index >= 0 && index < kNumKeys)
        {
            const auto i = static_cast<size_t>(index);
            keys_[i].isPressed = false;
            repaint(keys_[i].bounds);
        }
    }

    void Keyboard::calculateKeyBounds()
    {
        const auto area = getLocalBounds();
        const auto whiteKeyWidth = area.getWidth() / 52.0f;
        const auto blackKeyWidth = whiteKeyWidth * 0.6f;
        const auto whiteKeyHeight = area.getHeight();
        const auto blackKeyHeight = whiteKeyHeight * 0.6f;

        auto whiteKeyIndex = 0;

        for (size_t i = 0; i < kNumKeys; ++i)
        {
            const auto keyNumber = kFirstKey + static_cast<int>(i);
            auto& key = keys_[i];

            if (!key.isBlackKey)
            {
                const auto x = static_cast<int>(whiteKeyIndex * whiteKeyWidth);
                key.bounds = juce::Rectangle<int>(x, 0, static_cast<int>(whiteKeyWidth), whiteKeyHeight);
                ++whiteKeyIndex;
            }
            else
            {
                const auto noteFromA = (keyNumber - 21) % 12;
                const auto octave = (keyNumber - 21) / 12;
                const auto whiteKeysInOctave = octave * 7;

                auto leftWhiteKeyIndex = 0;
                switch (noteFromA)
                {
                    case 1:
                        leftWhiteKeyIndex = whiteKeysInOctave + 0;
                        break;
                    case 4:
                        leftWhiteKeyIndex = whiteKeysInOctave + 2;
                        break;
                    case 6:
                        leftWhiteKeyIndex = whiteKeysInOctave + 3;
                        break;
                    case 9:
                        leftWhiteKeyIndex = whiteKeysInOctave + 5;
                        break;
                    case 11:
                        leftWhiteKeyIndex = whiteKeysInOctave + 6;
                        break;
                }

                const auto blackKeyX = (leftWhiteKeyIndex + 1) * whiteKeyWidth - blackKeyWidth * 0.5f;

                key.bounds = juce::Rectangle<int>(static_cast<int>(blackKeyX), 0, static_cast<int>(blackKeyWidth),
                                                  static_cast<int>(blackKeyHeight));
            }
        }
    }

    int Keyboard::getKeyFromPosition(const juce::Point<int>& position) const
    {
        for (int i = kNumKeys - 1; i >= 0; --i)
        {
            const auto index = static_cast<size_t>(i);
            if (keys_[index].bounds.contains(position))
            {
                if (keys_[index].isBlackKey)
                    return kFirstKey + i;
            }
        }

        for (size_t i = 0; i < kNumKeys; ++i)
        {
            if (keys_[i].bounds.contains(position) && !keys_[i].isBlackKey)
                return kFirstKey + static_cast<int>(i);
        }

        return -1;
    }


    void Keyboard::notifyKeyPressed(int keyNumber)
    {
        listeners_.call([keyNumber](Listener& l) {
            l.keyPressed(keyNumber);
        });
    }

    void Keyboard::notifyKeyReleased(int keyNumber)
    {
        listeners_.call([keyNumber](Listener& l) {
            l.keyReleased(keyNumber);
        });
    }

}