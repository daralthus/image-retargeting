#pragma once

namespace IRL
{
    // most general stupid fallback
    template<class To, class From>
    void Convert(To& to, const From& from)
    {
        to = (To)from; 
    }

    // Other conversions in ***Conversion.h files
}