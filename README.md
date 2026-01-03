**Welcome to Flashcard!**  
# Getting the source code  
The source code can be cloned from GitHub using:
`git clone https://github.com/ThinkOutsideTheScope/flashcard.git`  
## Building the source code  
Flashcard is meant to be cross-compiled from Linux to Windows or Linux to Linux, although the source code itself it mutable, so this could probably become more flexible in the future, and the ucrt64 subdirectory contains a full MSYS2 UCRT64 build environment, so this could probably be build on Windows with some minor adjustments.
----
# OS-independent source code setup
This part of the setup must be run across all build targets before the actual build. This means it must be run first, then the OS-specific build should be run afterwards.
To setup the build, run:
`make decompress_ucrt64`
Note that this depends on the presence of `tar` and `xz` and should only be run once per source clone.
----
To build the source code, run:
`x86_64-w64-mingw32-make`
----
After the build completes, the `out` directory now contains flashcard.exe and all of its dependencies. This means that the `out` directory can now be copied to most other systems and will run just fine without anything else.  
# Running the output binary  
To run the binary on Linux, run:  
`wine out/flashcard.exe`  
----
> Note that this requires `wine` to run, and optionally some `winetricks` fonts to be able to view the text normally. On some systems, running
`GDK_BACKEND="win32" wine out/flashcard.exe`
is required to fix some compatibility issues with the app not starting (this should be used if you get the error "No such backend: ..."), and on some systems
`GSK_RENDERER="cairo" wine out/flashcard.exe`
fixes a problem where the app starts in a glitched state halfway off the screen, especially on Wayland.
----
To run the binary on Windows, simply run:  
`out\flashcard.exe`
# Build dependencies  
Although GTK 4 comes pre-built in the `ucrt64` direcotry, the source code does require some dependencies. These include:
- The x86_64-w64-mingw32-related toolkit, mostly: 
    1. x86_64-w64-mingw32-make
    2. x86_64-w64-mingw32-g++
	3. glib-compile-resources
		glib-compile-resources is part of the package:
			libglib2.0-dev on Debian/Ubuntu
			glib2-devel on Fedora/RHEL/Rocky/Alma/openSUSE
			glib2 on Arch Linux
