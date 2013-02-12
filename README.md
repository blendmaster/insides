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

You can use the texture files at [volvis.org](http://www.volvis.org). Try the
bonsai model:

    $ insides bonsai.raw 256 256 256

I've tuned some different transfer functions for selected volvis models. Edit
`fragment.glsl` and uncomment the named code block in `colorAt` for results
like the screenshots below. Yeah, I know it's not a great way to change the
transfer functions, but I couldn't think of a better method.

## Screenshots

![bonsai tree](http://i.imgur.com/gYGHUnB.png)
![engine](http://i.imgur.com/Q4YTH82.png)
![skull](http://i.imgur.com/gMQNlDS.png)

## Hacking

I spent some time adding some `inotify`-based live reloading of `fragment.glsl`
and `vertex.glsl`. While `insides` is running, you can edit and save the
shader code and see the results in the running program almost immediately.
Cool!

Unfortunately, this means that `insides` won't compile on non-POSIX systems. Oh
well.

