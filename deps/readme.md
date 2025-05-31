## raylib
- for rendering, windowing, input

## libtiff
- for loading tif images with LZW compression (like gebco_08_rev_elev_A1_grey_geo.tif)
- libgeotiff not worth using.  just need to define few tiff tag values.

## stb
- header-only; no compile
- for stb_image (png, jpg)
- no tif

## qoi
- for testing different image format sizes vs load times; may not use it long term now that I made .raw
- header-only; no compile

## cimgui
- C bindings for dear imgui (C++)
- has dear imgui as a submodule
- cimgui mirrors dear imgui's tags, so check that they match
- function prefix "ig"

## rlImGui
- makes it easy to use dear imgui with raylib

## marble_data
- contains the third party data for this project (color and topography images)
- easier than building TLS+SSL from source to download during build or first startup via HTTPS
- but very large files, so they're stored in a separate repo to preserve this repo's historical size

