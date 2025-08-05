#pragma once

#include <vector>

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace rp::uicore
{

    struct FunctionNode
    {
        float x, y;
        
        FunctionNode(float xPos, float yPos)
            : x(xPos), y(yPos)
        {
        }
    };

    class FunctionEditor : public juce::Component
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;
            virtual void nodeClicked(size_t index, float normalizedX, float normalizedY) = 0;
            virtual void nodeDragged(size_t index, float normalizedX, float normalizedY) = 0;
            virtual void nodeAdded(float normalizedX, float normalizedY) = 0;
            virtual void nodeDeleted(size_t index) = 0;
        };

        FunctionEditor();
        ~FunctionEditor() override = default;

        void setFunctionData(const std::vector<FunctionNode>& nodes);

        void addListener(Listener* listener);
        void removeListener(Listener* listener);

    private:
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseUp(const juce::MouseEvent& event) override;

        void renderGrid(juce::Graphics& g);
        void renderNodes(juce::Graphics& g);

        juce::Point<float> componentToNormalized(float componentX, float componentY) const;
        juce::Point<float> normalizedToComponent(float normalizedX, float normalizedY) const;

        int findNodeAt(float normalizedX, float normalizedY) const;

        std::vector<FunctionNode> nodes_;

        int draggedNodeIndex_;
        bool isDraggingNode_;
        juce::Point<float> lastMousePos_;

        juce::ListenerList<Listener> listeners_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FunctionEditor)
    };

}