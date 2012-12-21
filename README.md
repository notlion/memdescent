# Memory Descent

Memory overflow voxel raymarcher.

### About

The goal of this project was to visualize my CPU's memory as voxels. Initially, I thought it might be impossible because OS X seems to zero malloc'd memory.

To get around this, I created a pointer and just allocated a texture starting there, which seems to work reliably for 64^3 pixels. Any higher and the application crashes, I guess because it's accessing memory outside its own process. If anyone knows a more reliable way to grab large chunks of uninitialized memory, please let me know!

Once the data is in a texture, a shader is run on a full-screen quad. The texture is interpreted as a repeating 3-dimensional array and raymarched into using Amanatides and Woo's 1987 [voxel traversal algorithm](http://www.cse.yorku.ca/~amana/research/grid.pdf).

![Screenshot](http://farm9.staticflickr.com/8074/8289682921_4ce788315f_b.jpg)

This project was created at Art Hack Day SF 2012. The installed version used an Xbox USB controller.
