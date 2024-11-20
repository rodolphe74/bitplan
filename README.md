# bitplan
Displaying modern image formats on Atari ST.<br/>
Allow to load what [stb](https://github.com/nothings/stb) loads on an Atari ST.<br/>
Images are converted to 16 colors 320x200 pixels.

 <img src="images/bitplanes.svg" height="300"> 


 <img src="hicolor.jpg" width="400"> &nbsp; <img src="loading.gif" width="400">

# compilation
Using [FX_JavaDevBlog gcc68k Docker Image](https://www.fxjavadevblog.fr/atari-st-c-compiler-avec-docker/).

```shell
docker pull fxrobin/m68k-compiler
docker run --platform linux/amd64 -it -v $(pwd):/app fxrobin/m68k-compiler bash
make
```
Then create a GEMDOS drive to the git folder under Hatari.<br/>
Git folder should be available under drive C.
