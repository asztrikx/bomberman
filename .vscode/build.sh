#!/bin/bash
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )""/../"
location=$(pwd)'/'
regex='s:./:'$location':'

gcc \
-g \
-Wall \
-pedantic \
-lm \
-Werror \
`sdl2-config --cflags --libs` \
\
-lSDL2_gfx \
-lSDL2_ttf \
-lSDL2_image \
-lSDL2_mixer \
\
main.c \
$(find . -name '*.c' | sed 's:./main.c::' | sed $regex | tr '\n' ' ') \
\
-o \
main \
