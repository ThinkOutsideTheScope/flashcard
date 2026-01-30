# **Welcome to Flashcard!**

## Flashcard is a simple studying app that uses GTK 4 and can be built with a few easy steps.

### Building Flashcard

If you have not already, clone the repository with `git clone https://github.com/ThinkOutsideTheScope/flashcard.git` and `cd flashcard`
Find and copy/symlink the `gtk4.pc` file to the root directory of the git clone
Now, run `make` or (more recommended) `make -j$(nproc)`
The dependencies for this are as follows:
1. gcc
2. g++
3. make
4. pkg-config
5. glib-compile-resources
6. A GTK 4.18+ development environment, and also a GTK 4 runtime environment if you will run this app in the same environment. 
The exact packages needed for these vary between distributions

### Running Flashcard

Once flashcard has been built and a GTK 4 runtime environment has been installed, simply run `out/flashcard` in the flashcard clone directory. The executable can be copied to any other directory in the system, as it only depends on GTK 4 runtime once built.
