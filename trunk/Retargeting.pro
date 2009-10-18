CONFIG += qt console
debug {
  CONFIG += debug
}
SOURCES += main.cpp

PRECOMPILED_HEADER = Includes.h

HEADERS += pstdint.h Config.h RefCounted.h CopyOnWrite.h
HEADERS += Convert.h Accumulator.h TypeTraits.h Random.h Rectangle.h

HEADERS += IO.h
SOURCES += IO.cpp

HEADERS += Profiler.h
SOURCES += Profiler.cpp

HEADERS += Threading.h ThreadingQt.h Parallel.h
SOURCES += Parallel.cpp

HEADERS += Point2D.h
SOURCES += Point2D.cpp

HEADERS += RGB.h Lab.h ColorConversion.h

HEADERS += Image.h ImageConversion.h
HEADERS += Scaling.h
HEADERS += GaussianPyramid.h

HEADERS += NearestNeighborField.h