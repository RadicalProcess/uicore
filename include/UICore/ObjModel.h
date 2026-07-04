#pragma once

#include <juce_core/juce_core.h>

#include <array>
#include <vector>

namespace rp::uicore
{

    // Loads a Wavefront .obj mesh (vertex positions and faces only) and exposes
    // the geometry as a flat list of triangles ready for rendering. Faces with
    // more than three vertices are triangulated with a simple fan, so both
    // triangle and quad based models are supported. Normals, texture
    // coordinates and materials present in the source file are ignored.
    class ObjModel
    {
    public:
        using Position = std::array<float, 3>;

        struct Triangle
        {
            Position a;
            Position b;
            Position c;
        };

        ObjModel();

        // Parses OBJ text. Returns true when at least one triangle was produced.
        bool loadFromString(const juce::String& text);

        // Loads and parses an OBJ file from disk. Returns true on success.
        bool loadFromFile(const juce::File& file);

        bool isEmpty() const;

        const std::vector<Triangle>& getTriangles() const;

        // Axis-aligned bounding box of the loaded vertices. Both bounds are the
        // origin while the model is empty.
        Position getMinBounds() const;
        Position getMaxBounds() const;

        // Centre of the bounding box.
        Position getCentre() const;

        // Largest bounding-box dimension across the three axes.
        float getMaxExtent() const;

    private:
        void computeBounds(const std::vector<Position>& positions);

        std::vector<Triangle> triangles_;
        Position minBounds_;
        Position maxBounds_;
    };

}
