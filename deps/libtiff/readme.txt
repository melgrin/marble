This is a copy of libtiff v4.7.0.  Its build has been modified to a) avoid cmake and b) reduce the feature set to just support loading LZW-compressed TIFFs, because that's the only form I need.  This removes dependencies on all other libraries.

tif_config.h, tiffconf.h, and tiffvers.h were originally produced by separately building libtiff with cmake with a bunch of flags set (to disable the other compression types and reduce dependencies, as mentioned above).

cmake -DCMAKE_INSTALL_PREFIX=../install -Dtiff-tools=OFF -Dcxx=OFF -Dtiff-opengl=OFF -Dtiff-tools=OFF -Dlibdeflate=OFF -Djbig=OFF -Djpeg=OFF -Dold-jpeg=OFF -Djpeg12=OFF -Dlerc=OFF -Dlzma=OFF -Dpixarlog=OFF -Dsphinx=OFF -Dwebp=OFF -Dzlib=OFF -Dzstd=OFF
cmake --build . --config Release --target install
ctest . -C Release


# internalcodecs
ccitt
packbits
lzw
thunder
next
logluv
mdi

