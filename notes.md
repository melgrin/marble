https://visibleearth.nasa.gov/grid
https://visibleearth.nasa.gov/collection/1484/blue-marble
https://visibleearth.nasa.gov/collection/1723/visible-earth

https://visibleearth.nasa.gov/images/73934/topography
"Data in these images were scaled 0-6400 meters."

https://visibleearth.nasa.gov/images/147190/explorer-base-map
https://visibleearth.nasa.gov/images/144898/earth-at-night-black-marble-2016-color-maps



//Image color_image_full = load_image("local/world.200405.3x21600x21600.A1.png"); // stbi_load fails with error "Image too large to decode".  looks like it can only load PNGs with decompressed would-be size of ~1 GB or less.
stbi bmp expands 3-component images to 4-component, so it is slow on large images:
//   PNG creates output files with the same number of components as the input.
//   The BMP format expands Y to RGB in the file format and does not
//   output alpha.




//ImageCrop(&color_image, crop); // this frees the original image.data
//Image color_image = ImageFromImage(color_image_full, crop); // mallocs new image, retains original

