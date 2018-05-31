cake.obj : cake.c cake.h
     cc -d -c -u -W2 -Asnw -Gsw -Os -Zpe cake.c

cake.res: cake.rc cake.h
     rc -r cake.rc

cake.exe : cake.obj cake.res cake.def
     link4 cake, /align:16, /map, slibw swlibc/NOD, cake
     mapsym cake
     rc cake.res
