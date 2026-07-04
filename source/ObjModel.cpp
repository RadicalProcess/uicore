#include "UICore/ObjModel.h"

#include <algorithm>
#include <charconv>
#include <sstream>
#include <string>

namespace rp::uicore
{

    namespace
    {
        bool parseIndex(const std::string& token, int vertexCount, int& indexOut)
        {
            // OBJ face vertices may carry texture / normal references separated
            // by '/'. Only the leading position index is relevant here.
            const auto slash = token.find('/');
            const auto positionToken = (slash == std::string::npos) ? token : token.substr(0, slash);

            if (positionToken.empty())
                return false;

            auto value = 0;
            const auto* begin = positionToken.data();
            const auto* end = positionToken.data() + positionToken.size();
            if (std::from_chars(begin, end, value).ec != std::errc())
                return false;

            // OBJ indices are 1-based; negative values count back from the most
            // recently defined vertex.
            const auto resolved = (value < 0) ? vertexCount + value : value - 1;
            if (resolved < 0 || resolved >= vertexCount)
                return false;

            indexOut = resolved;
            return true;
        }
    }

    ObjModel::ObjModel()
    : minBounds_({ 0.0f, 0.0f, 0.0f })
    , maxBounds_({ 0.0f, 0.0f, 0.0f })
    {
    }

    bool ObjModel::loadFromString(const juce::String& text)
    {
        triangles_.clear();

        auto positions = std::vector<Position>{};

        auto stream = std::istringstream(text.toStdString());
        auto line = std::string{};

        while (std::getline(stream, line))
        {
            auto lineStream = std::istringstream(line);
            auto prefix = std::string{};
            lineStream >> prefix;

            if (prefix == "v")
            {
                auto position = Position{ 0.0f, 0.0f, 0.0f };
                lineStream >> position[0] >> position[1] >> position[2];
                positions.push_back(position);
                continue;
            }

            if (prefix != "f")
                continue;

            auto faceIndices = std::vector<int>{};
            auto token = std::string{};
            while (lineStream >> token)
            {
                auto index = 0;
                if (! parseIndex(token, static_cast<int>(positions.size()), index))
                {
                    faceIndices.clear();
                    break;
                }

                faceIndices.push_back(index);
            }

            // Fan triangulation of the (possibly n-sided) face.
            for (auto i = size_t{ 2 }; i < faceIndices.size(); ++i)
            {
                triangles_.push_back({ positions[static_cast<size_t>(faceIndices[0])],
                                       positions[static_cast<size_t>(faceIndices[i - 1])],
                                       positions[static_cast<size_t>(faceIndices[i])] });
            }
        }

        computeBounds(positions);

        return ! triangles_.empty();
    }

    bool ObjModel::loadFromFile(const juce::File& file)
    {
        if (! file.existsAsFile())
            return false;

        return loadFromString(file.loadFileAsString());
    }

    bool ObjModel::isEmpty() const
    {
        return triangles_.empty();
    }

    const std::vector<ObjModel::Triangle>& ObjModel::getTriangles() const
    {
        return triangles_;
    }

    ObjModel::Position ObjModel::getMinBounds() const
    {
        return minBounds_;
    }

    ObjModel::Position ObjModel::getMaxBounds() const
    {
        return maxBounds_;
    }

    ObjModel::Position ObjModel::getCentre() const
    {
        return { 0.5f * (minBounds_[0] + maxBounds_[0]),
                 0.5f * (minBounds_[1] + maxBounds_[1]),
                 0.5f * (minBounds_[2] + maxBounds_[2]) };
    }

    float ObjModel::getMaxExtent() const
    {
        const auto extentX = maxBounds_[0] - minBounds_[0];
        const auto extentY = maxBounds_[1] - minBounds_[1];
        const auto extentZ = maxBounds_[2] - minBounds_[2];
        return std::max(extentX, std::max(extentY, extentZ));
    }

    void ObjModel::computeBounds(const std::vector<Position>& positions)
    {
        minBounds_ = { 0.0f, 0.0f, 0.0f };
        maxBounds_ = { 0.0f, 0.0f, 0.0f };

        if (positions.empty())
            return;

        minBounds_ = positions.front();
        maxBounds_ = positions.front();

        for (const auto& position : positions)
        {
            for (auto axis = size_t{ 0 }; axis < 3; ++axis)
            {
                minBounds_[axis] = std::min(minBounds_[axis], position[axis]);
                maxBounds_[axis] = std::max(maxBounds_[axis], position[axis]);
            }
        }
    }

}
