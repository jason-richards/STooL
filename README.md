# STooL    

STooL is a "Swiss Army Knife" tool for STL files.  Quickly Scale, Rotate, Split and Translate objects in STL files.


## Requirments                                          
- Favorite Linux distro
- C++ 20
- Boost


## Building                                                                                         
```bash
# Release Build
make all    

# Debug Build
make debug

# Clean
make all
```


## Usage                                                 

#### DEBUG: dump STL file.
```bash ./stool --input input.stl --dump```


#### ROTATE: X axis 90 degrees, Y axis 45 degrees, Z axis 0.
```bash ./stool --input input.stl --output output.stl --rotate 90,45,0```


#### SCALE: X axis 25%, Y axis 2X
```bash ./stool --input input.stl --output output.stl --scale 0.25,2,1```


#### SPLIT: separate "Manifold" objects into separate files.
```bash ./stool --input input.stl --split```


#### TRANSLATE: move objects within STL file.
```bash ./stool --input input.stl --output output.stl --translate 10,1,-3.3```


#### CENTROID: calculate the center of mass the STL.
```bash ./stool --input input.stl --centroid```


#### MIN/MAX: calculate the maximum and minimum of each axis.
```bash ./stool --input input.stl --minmax```


## License
[MIT](https://choosealicense.com/licenses/mit/)
