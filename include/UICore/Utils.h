#pragma once

#include <iomanip>

namespace rp::uicore
{
    inline
    std::string reduceNumDecimals(float value, size_t numDecimals)
    {
        auto stream = std::stringstream();
        stream << std::fixed << std::setprecision(static_cast<int>(numDecimals)) << value;
        return stream.str();
    }
}
