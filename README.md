# wish

wish (Wisconsin Shell) is a simple UNIX-like shell made to understand about creation/destruction of processes and the working of shell. It is one of the projects from the book [Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/)

wish supports
- Built-in commands- `cd`, `path`, `printpath` and `exit`.
- Parallel commands
- Output redirection

It passes all the tests (except one, for some reason). To build wish simply `make wish`. Then wish can be run in interactive mode using `./wish` or batch mode using `./wish batch.txt` which takes an input file to run commands. To run tests, simply run `./run-tests.sh`.

The detailed project description can be found [here](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/processes-shell)

