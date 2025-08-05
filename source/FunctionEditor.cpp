#include <UICore/FunctionEditor.h>

#include <algorithm>
#include <cmath>

namespace rp::uicore
{

    FunctionEditor::FunctionEditor()
    : draggedNodeIndex_(-1)
    , isDraggingNode_(false)
    {
        setOpaque(true);

        nodes_.emplace_back(0.0f, 0.0f);
        nodes_.emplace_back(1.0f, 1.0f);
    }

    void FunctionEditor::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colour(25, 25, 25));

        g.setColour(juce::Colour(50, 50, 50));
        g.drawRect(getLocalBounds(), 1);

        renderGrid(g);
        renderNodes(g);
    }

    void FunctionEditor::resized()
    {
        repaint();
    }

    void FunctionEditor::mouseDown(const juce::MouseEvent& event)
    {
        const auto normalizedPos = componentToNormalized(static_cast<float>(event.x), static_cast<float>(event.y));
        lastMousePos_ = normalizedPos;

        const auto nodeIndex = findNodeAt(normalizedPos.x, normalizedPos.y);

        if (nodeIndex >= 0)
        {
            if (event.mods.isShiftDown())
            {
                listeners_.call([nodeIndex](Listener& l) {
                    l.functionNodeDeleted(static_cast<size_t>(nodeIndex));
                });
            }
            else
            {
                isDraggingNode_ = true;
                draggedNodeIndex_ = nodeIndex;

                listeners_.call([nodeIndex, &normalizedPos](Listener& l) {
                    l.functionNodeClicked(static_cast<size_t>(nodeIndex), normalizedPos.x, normalizedPos.y);
                });
            }
        }
        else
        {
            listeners_.call([&normalizedPos](Listener& l) {
                l.functionNodeAdded(normalizedPos.x, normalizedPos.y);
            });
        }

        repaint();
    }

    void FunctionEditor::mouseDrag(const juce::MouseEvent& event)
    {
        const auto normalizedPos = componentToNormalized(static_cast<float>(event.x), static_cast<float>(event.y));

        if (isDraggingNode_ && draggedNodeIndex_ >= 0)
        {
            auto newX = normalizedPos.x;
            auto newY = std::clamp(normalizedPos.y, 0.0f, 1.0f);

            const auto nodeIndex = static_cast<size_t>(draggedNodeIndex_);
            const auto isFirstNode = (nodeIndex == 0);
            const auto isLastNode = (nodeIndex == nodes_.size() - 1);

            if (isFirstNode)
            {
                newX = 0.0f;
            }
            else if (isLastNode)
            {
                newX = 1.0f;
            }
            else
            {
                newX = std::clamp(newX, 0.0f, 1.0f);
            }

            listeners_.call([nodeIndex, newX, newY](Listener& l) {
                l.functionNodeDragged(nodeIndex, newX, newY);
            });
        }

        lastMousePos_ = normalizedPos;
        repaint();
    }

    void FunctionEditor::mouseUp(const juce::MouseEvent& /*event*/)
    {
        isDraggingNode_ = false;
        draggedNodeIndex_ = -1;
    }

    void FunctionEditor::setFunctionData(const std::vector<FunctionNode>& nodes)
    {
        nodes_ = nodes;
        repaint();
    }

    void FunctionEditor::addListener(Listener* listener)
    {
        listeners_.add(listener);
    }

    void FunctionEditor::removeListener(Listener* listener)
    {
        listeners_.remove(listener);
    }

    void FunctionEditor::renderGrid(juce::Graphics& g)
    {
        const auto bounds = getLocalBounds().toFloat();
        g.setColour(juce::Colour(60, 60, 60));

        const auto numLines = 8;
        for (int i = 1; i < numLines; ++i)
        {
            const auto y = bounds.getHeight() * static_cast<float>(i) / static_cast<float>(numLines);
            g.drawLine(0.0f, y, bounds.getWidth(), y, 0.5f);

            const auto x = bounds.getWidth() * static_cast<float>(i) / static_cast<float>(numLines);
            g.drawLine(x, 0.0f, x, bounds.getHeight(), 0.5f);
        }
    }

    void FunctionEditor::renderNodes(juce::Graphics& g)
    {
        for (size_t i = 0; i < nodes_.size(); ++i)
        {
            const auto& node = nodes_[i];
            const auto isEdgeNode = (i == 0 || i == nodes_.size() - 1);
            const auto radius = 4.0f;
            const auto componentPos = normalizedToComponent(node.x, node.y);

            if (isEdgeNode)
                g.setColour(juce::Colour(255, 100, 100));
            else
                g.setColour(juce::Colour(100, 255, 100));

            g.fillEllipse(componentPos.x - radius, componentPos.y - radius, radius * 2, radius * 2);

            g.setColour(juce::Colours::white);
            g.drawEllipse(componentPos.x - radius, componentPos.y - radius, radius * 2, radius * 2, 1.0f);
        }
    }

    juce::Point<float> FunctionEditor::componentToNormalized(float componentX, float componentY) const
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto normalizedX = componentX / bounds.getWidth();
        const auto normalizedY = 1.0f - (componentY / bounds.getHeight());
        return juce::Point<float>(normalizedX, normalizedY);
    }

    juce::Point<float> FunctionEditor::normalizedToComponent(float normalizedX, float normalizedY) const
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto componentX = normalizedX * bounds.getWidth();
        const auto componentY = (1.0f - normalizedY) * bounds.getHeight();
        return juce::Point<float>(componentX, componentY);
    }

    int FunctionEditor::findNodeAt(float normalizedX, float normalizedY) const
    {
        const auto tolerance = 0.05f;

        for (size_t i = 0; i < nodes_.size(); ++i)
        {
            const auto& node = nodes_[i];
            const auto dx = node.x - normalizedX;
            const auto dy = node.y - normalizedY;
            const auto distance = std::sqrt(dx * dx + dy * dy);

            if (distance < tolerance)
                return static_cast<int>(i);
        }

        return -1;
    }

}