#include "TrajectoryThumbnail.h"
#include "Style.h"

namespace rp::uicore
{
    TrajectoryThumbnail::TrajectoryThumbnail()
    {
    }

    void TrajectoryThumbnail::paint(juce::Graphics &g)
    {
        g.fillAll(styles::canvasBackground);
    }

}
