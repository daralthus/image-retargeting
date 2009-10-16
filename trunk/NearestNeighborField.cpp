#include "Includes.h"
#include "NearestNeighborField.h"

namespace IRL
{
    // enable conversion from Offset to RGB8 for visualization purposes
    template<>
    void Convert(RGB8& to, const Offset& from)
    {
        const double Pi = 3.1415926535897932384626433832795;
        double len = sqrt((double)(from.dx * from.dx + from.dy * from.dy));
        double dir = (from.dx == 0 ? Pi * 0.5 : atan(abs((double)from.dy / from.dx)));
        if (from.dx >= 0 && from.dy >= 0) dir += 0;           // I
        if (from.dx <  0 && from.dy >= 0) dir = Pi - dir;     // II
        if (from.dx <  0 && from.dy <  0) dir = Pi + dir;     // III
        if (from.dx >= 0 && from.dy <  0) dir = 2 * Pi - dir; // IV
        dir = dir * 360 / (2 * Pi);

        // HSV
        double h = dir;
        double s = len / 40; // normalized
        double v = 0.8;

        if (s > 1.0) s = 1.0;

        // HSV -> RGB
        // see http://en.wikipedia.org/wiki/HSL_color_space
        int h_i = ((int)floor(h) / 60) % 6;
        double f = h / 60 - h_i;
        double p = v * (1 - s);
        double q = v * (1 - f * s);
        double t = v * (1 - (1 - f) * s);

        double r, g, b;
        switch (h_i)
        {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
        }

        int Rb = (int)(r * 255); if (Rb < 0) Rb = 0; else if (Rb > 255) Rb = 255;
        int Gb = (int)(g * 255); if (Gb < 0) Gb = 0; else if (Gb > 255) Gb = 255;
        int Bb = (int)(b * 255); if (Bb < 0) Bb = 0; else if (Bb > 255) Bb = 255;

        to.R = (uint8_t)Rb;
        to.G = (uint8_t)Gb;
        to.B = (uint8_t)Bb;
    }
}