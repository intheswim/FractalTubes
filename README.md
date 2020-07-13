#### Some backgroud. 

<pre> 

The  original  version of this program, "M3DSaver" (stands for Mandelbrot in 3D)
was written in  summer of 2003 for Windows  XP. It was  drawing static images of
tubes "painted" with  fractals (Mandelbrot  or Julia  sets).  A version  of this
program still works, amazingly, under Windows 10, in 2020, as this file is being
written, but parts  of it look and feel out of date. This version was using only
Windows API for drawing tubes (pixel by  pixel) and was extremely lighweight. It
also included  a dialog to  select and  create/add new tube decorations based on
user input - in a separate window with fractal editing tool. 

Fast forward to 2015,  this program, renamed into "Fractal Tubes", was rewritten
to show dynamic (moving)  tubes using OpenGL. First for MacOS, then  for Windows
7. 

</pre> 

[Fractal Tubes site](http://fractaltubes.com) 

A screenshot of the original screensaver:

![Screenshot](/shot_a1.jpg)

<pre> 

Somewhere  around  2017-18  Apple (sadly) removed  OpenGL from  its  development
tools, replacing it with its own graphics framework. 

2020  I  am  writing Linux version for xscreensaver and  open-sourcing  it. I am
removing all  Windows- and  MacOS-specific code, doing  some cleanup and  adding
comments - overall this program is similar to 2015 version. 

</pre> 

#### To build and install 

<pre> 

Prerequisites are X11 and OpenGL. To install X11: 

`sudo apt-get update -y sudo apt-get install -y libx11-dev` 

To install GLUT (which installs OpenGL as well): 

`sudo apt-get install freeglut3 freeglut3-dev` 

Note: I  also  experimented with  making  it  using  GLUT  framework,  but  only
succeeded in making  it  run  as  standaline application (not as  xscreensaver).
Details are in main_glut.cpp 

To build the GLUT version, type: 

`make glut=1` 

(it will produce m3d_glut, which you can run from command line). 

</pre> 

#### To run as xscreensaver 

<pre> 

First, install xscreensaver, a well known package that implements screensavers 
for Linux: 

`sudo apt install xscreensaver`. 

The xscreensaver documentation by its author can be found here: 

[XScreensaver Manual](https://www.jwz.org/xscreensaver/man1.html) 

Now, we can use Fractal Tubes: 

1. To run as standalone application, simply type `m3d` after building it. 

2. To install as xscreensaver, run make with `install` target: 

`sudo make install` 

then type: 

`xscreensaver-demo` 

Pick any included screensaver  in the list and press "Settings"  then "Advanced"
and in "Command line" type 

`m3d -root` 

in  place  of existing  screensaver (don't  worry,  the  old one  still  will be
preserved). Then click OK to save and exit. Next time you open xscreensaver-demo
`M3d` saver should be listed and you can make it your default screensaver. 

</pre> 

![Xscreensaver preview screen](/preview.png)

![M3D settings screen within xscreensaver](/settings.png)

#### Ideas for improvement 

<pre> 

A possible addition would be to include a custom tube editor that was present in
2003 version - in form  of a  separate application or as a window  which  can be
opened with a command line option. 

</pre> 

#### License 

[MIT](https://choosealicense.com/licenses/mit/) 





