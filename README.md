# The Witness HD Mod
This mod patches The Witness (2016) binary to make the circles on panels have more vertices.

## Recommended Secret Settings
The Witness exposes some secret settings that you can change by adding them to the data/Local.variables file in The Witness install directory
Some of these work better without any changes, but others can cause issues. I will document what I know here.

### Working Without Issue (AFAIK)
- point_shadow_resolution 4096
- cluster_distance 10000.0
- lod_distance 100000.0
- grass_fade_begin 1000
- grass_fade_end 1500
### Work But Require Future Mod Patches
- panel_render_width: Technically works fine, but makes polyminos very thin and barely visible
- sun_shadow_resolution: Near rendering works fine, but it seems to break the bounds that get rendered to the shadow map, causing middle to far distance objects to not be shadowed
### Work But Change Gameplay
- panel_fade_begin/panel_fade_end: Causes some EPs to be less-continuous in colour

To use or play around with any of these add these lines to your ```data/Local.variables``` file and select ```high``` in the launcher
```
#You can comment/uncomment lines with '#'
:/render/high
panel_render_width 4096
point_shadow_resolution 4096
cluster_distance 10000.0
lod_distance 100000.0
grass_fade_begin 1000
grass_fade_end 1500
panel_fade_begin 50.0
panel_fade_end 65.0
```

## Future Planned Features:
- A fix for the rendering bounds error when increasing sun_shadow_map_resolution
- A fix for subtraction polyminos becoming very thin and barely visible when panel_render_width is increased


