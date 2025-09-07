- github actions extras
  - TODO actions/stale@v9, or see if there's an "expires" setting with upload (there is, see upload-artifact: with: retention-days:).  this is also apparently a "repository setting", so I could look into that.
  - TODO? schedule: cron:
  - add `workflow_dispatch`.  this enables button on web ui to manually run a workflow.  it must be in the workflow yml on the default branch (main)

- maybe resize topo *up* to 21600x21600 instead of resizing bmng *down* to 10800x10800 to avoid quality loss

- manage image sizes intelligently?
  - lots of hardcoded 10800 and 21600 around
  - can read out of the file metadata via `stb_info`

- build doesn't need to generate .raw via imgconv.exe anymore.  it's in main.c now.

- depth 1 yet recursive submodule clone?

- dll separation between raylib and my code because things like stb and Windows (you might have heard of it, but raysan has not) tend to clash

- ui
  - debug pressing escape from within input field then ui defocus button causes two problems:  1) teleports view 2) keeps mouse visible, which causes camera to not be able to turn freely.  something to do with GLFW cursor mode.  step through ~5 frames to see it  erroneously go from DISABLED to NORMAL.  not sure why its that many frames.  I think rlImGuiBegin is maybe seeing the cursor update from my DisableCursor call, and that causes it to Show/Hide.
  - can I get this process's ram and vram usage and display it? (no, you can only get overall vram usage, but you can get per-process ram, like task manager)
- minimap
  - image
  - camera position
  - camera rotation
  - current tiles
- debug crash when going to tile 0,0 via ui
- debug why there are gaps between tiles.  height map edge weird behavior?  example: lat 59.988696, lon -150.170750.  see ./local/tile_gap.png.
- debug why there seems to be 56 tiles, despite 10800 / 200 == 54.  seeing max tile y index of 55 and min tile y index of -1!  see ./local/off_the_end.png.
- first time downloads/setup from freshly-cloned repos
- background thread for tile loading.  cancellable when moving fast etc.
- lat/lon usually aren't 1:1 proportional, but I'm pretending they are; image stretched, at least slightly
- experiment with larger tile sizes or more tiles in pool
- figure out correct height for terrain
- externally-controlled position (via port, shmem, or similar)
- handling for all image grids (A1, C2, etc)
  - download all?
  - disk space vs load time?
  - region select/teleport?
- osm? borders, places, buildings
- place search?


- some visual issues with tile debug mode
  - blank black rectangle in the upper left of screen
  - black rectangles that show tile info disappear the moment the left edge goes off the screen

- dedup color_image_full and _color_full
- maybe use Img in GeoTIFFData

