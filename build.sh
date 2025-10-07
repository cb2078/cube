debug=1
options='-Wall -Wextra -Wpedantic -lSDL3 -lGL -lm *.c'
if test $debug -eq 1
then
    gcc $options -Wno-unused-function -Wno-unused-variable -Wno-parentheses -g
else
    gcc $options -O3
fi
