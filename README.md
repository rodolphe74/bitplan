# bitplan
Displaying modern image formats on Atari ST.

 <img src="images/bitplanes.svg" height="300"> 


 <img src="hicolor.jpg" width="400"> &nbsp; <img src="loading.gif" width="400">

# compilation
Using [FX_JavaDevBlog gcc68k Docker Image](https://www.fxjavadevblog.fr/atari-st-c-compiler-avec-docker/).

```shell
docker pull fxrobin/m68k-compiler
docker run --platform linux/amd64 -it -v $(pwd):/app fxrobin/m68k-compiler bash
make
```
