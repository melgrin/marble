if not exist local mkdir local
pushd local

:: https://visibleearth.nasa.gov/images/73934/topography
:: ~10-30 MB each
curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73934/gebco_08_rev_elev_A1_grey_geo.tif
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73934/gebco_08_rev_elev_A2_grey_geo.tif
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73934/gebco_08_rev_elev_B1_grey_geo.tif
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73934/gebco_08_rev_elev_B2_grey_geo.tif
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73934/gebco_08_rev_elev_C1_grey_geo.tif
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73934/gebco_08_rev_elev_C2_grey_geo.tif
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73934/gebco_08_rev_elev_D1_grey_geo.tif
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73934/gebco_08_rev_elev_D2_grey_geo.tif


:: https://visibleearth.nasa.gov/images/74042/may-blue-marble-next-generation
:: ~10-40 MB each
curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.A1.jpg
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.A2.jpg
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.B1.jpg
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.B2.jpg
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.C1.jpg
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.C2.jpg
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.D1.jpg
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.D2.jpg
:: ~100-400 MB each
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.A1.png
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.A2.png
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.B1.png
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.B2.png
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.C1.png
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.C2.png
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.D1.png
:: curl -f -O https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74042/world.200405.3x21600x21600.D2.png


:: TODO try these; they have relief shading

:: https://visibleearth.nasa.gov/images/73701/may-blue-marble-next-generation-w-topography-and-bathymetry
:: https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73701/world.topo.bathy.200405.3x21600x21600.A1.jpg
:: https://eoimages.gsfc.nasa.gov/images/imagerecords/73000/73701/world.topo.bathy.200405.3x21600x21600.A1.png

:: https://visibleearth.nasa.gov/images/74343/may-blue-marble-next-generation-w-topography
:: https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74343/world.topo.200405.3x21600x21600.A1.jpg
:: https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74343/world.topo.200405.3x21600x21600.A1.png

popd
