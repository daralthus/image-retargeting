#pragma once

#include "Convert.h"
#include "RGB.h"
#include "Lab.h"
#include "Alpha.h"

namespace IRL
{
    // Lab -> RGB
    template<class RgbChannel, class LabChannel>
    void Convert(RGB<RgbChannel>& rgb, const Lab<LabChannel>& lab);
 
    // RGB -> Lab
    template<class LabChannel, class RgbChannel>
    void Convert(Lab<LabChannel>& lab, const RGB<RgbChannel>& rgb);

    // RGB <-> RGB
    template<class ChannelTo, class ChannelFrom>
    void Convert(RGB<ChannelTo>& to, const RGB<ChannelFrom>& from);

    // Lab <-> Lab
    template<class ChannelTo, class ChannelFrom>
    void Convert(Lab<ChannelTo>& to, const Lab<ChannelFrom>& from);

    // Alpha <-> Alpha
    template<class ChannelTo, class ChannelFrom>
    void Convert(Alpha<ChannelTo>& to, const Alpha<ChannelFrom>& from);
}

// implementation file
#include "ColorConversion.inl"