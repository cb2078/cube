# codebase

- `cube.c`: A fast (but not optimal) representation of a Rubik's cube to be
used in a solver.

- `gui.c`: Code to create an SDL window for viewing the cube in 3D. There are
animations for turns and corner cutting. The cube can be turned with the
keyboard or from the api.

- `moves.c`: Library for moves (such as what face a move turns, how much it
turns) and algorithms (inverse, cancellations, reversing)

- `coord_meta.c`: Due to the limiations of C and the scope of this project
(supporting multiple solvers) writing code to index a coordate is either
tedious (you write several getters and setters for each coordinate) or slow
(writting a generic getter and setter given a coordinate description). The
workaround is to write a that generates fast C code (`coord.c`) to index
coordinates.

- `solver.c`: An implementation of IDA* with BPMX[1] optissation
