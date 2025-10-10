OPTIONS='-Wall -Wextra -Wpedantic -lSDL3 -lGL -lm *.c'
if test ! $RELEASE
then
    gcc $OPTIONS -Wno-unused-function -Wno-unused-variable -Wno-parentheses -g
else
    gcc $OPTIONS -O3
fi
