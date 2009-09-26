CONFIG += qt console
debug {
  CONFIG += debug
}
SOURCES += main.cpp

HEADERS += pstdint.h Assert.h Config.h
HEADERS += Lab.h Real.h Color.h

HEADERS += Image.h
SOURCES += Image.cpp