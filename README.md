# insides

Now you can see what stuff looks like... ON THE INSIDES! Well, that is, if
you're in to that sort of thing.

This is Steven Ruppert's Project 1 for **CSCI447
Scientific Visualization** at the Colorado School of Mines, in the
Spring 2013 semester.

## Usage

To compile, you'll need the GLUT, GLEW, and GLM libraries installed. Use your
Linux distro's package manager, that's the easiest way to get them.

Then, run:

    $ make

to create the `insides` executable. The command line arguments are:

    $ insides <texture file> <xsize> <ysize> <zsize>

You can use the texture files at [volvis.org](http://www.volvis.org). The
bonsai model works the best right now:

    $ insides bonsai.raw 256 256 256

## Screenshots

![bonsai tree](http://i.imgur.com/38JZrQl.png)
![engine](http://i.imgur.com/aTH9WiD.png)

## Hacking

I spent some time adding some `inotify`-based live reloading of `fragment.glsl`
and `vertex.glsl`. While `insides` is running, you can edit and save the
shader code and see the results in the running program almost immediately.
Cool!

Unfortunately, this means that `insides` won't compile on non-POSIX systems. Oh
well.

