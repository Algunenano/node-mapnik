# CARTO node-mapnik changelog

## 3.6.2-carto.11

**Release date**: 2018-XX-XX

Changes:
 - Use protozero from submodule instead of the deprecated npm module.

## 3.6.2-carto.10

**Release date**: 2018-05-14

Changes:
 - Update mapnik to [v3.0.15.9](https://github.com/CartoDB/mapnik/blob/v3.0.15.9/CHANGELOG.carto.md#30159) which fixes a bug when rendering zero lenght lines using the marker symbolizer.

## 3.6.2-carto.9

**Release date**: 2018-05-07

Changes:
 - Update mapnik to [v3.0.15.8](https://github.com/CartoDB/mapnik/blob/v3.0.15.8/CHANGELOG.carto.md#30158) which brings:
   - Move marker cache to be used per map instead of globally
   - Compatibility improvements (ICU 61, FreeType2 2.9.1)
   - Avoid building the collision tree when possible, also when using load_map_string()
 - Update node-pre-gyp to 0.10.0
 - Node 10 compatibility

## 3.6.2-carto.8

**Release date**: 2018-04-16

Changes:
 - Upgrade nan to "2.10.0" and node-pre-gyp to "0.7.0" for node 9 and 10 compatibility.
 - Use Carto's fork for mapnik-vector-tile. Uses `v1.6.1-carto.1` which is vanilla `1.6.1` plus a fix to avoid features with the same value but different type being stored together.

## 3.6.2-carto.7

**Release date**: 2018-04-02

Changes:
  - Upgrade mapnik to [v3.0.15.7](https://github.com/CartoDB/mapnik/blob/345a9670e14ab898b8abe44f240be0fb6c37cb98/CHANGELOG.carto.md#30157), which brings a number of changes:
    - Metrics: Add metrics to the grid renderer
    - Metrics: Internal rework (no mutex, no tree) to improve performance
    - Metrics: Add counters for feature types
    - Metrics: Add counters for cache misses
    - Markers Symbolizer Caches: Add a map extra parameter markers_symbolizer_caches_disabled that can be set to true to disable them.
    - Markers Symbolizer Caches: Use pointer to cached attributes as part of the key. Fix for a double free. Performance improvements in cached items keys lookup.

## 3.6.2-carto.6

**Release date**: 2018-03-19

Changes:
  - Replublish npm package including git submodules

## 3.6.2-carto.5

**Release date**: 2018-03-19

Changes:
 - [Upstream] move to prepublishOnly
 - [Upstream] Add mapnik-index command in 'bin' so it gets installed by npm
 - Allow switching the default build with an environment variable

## 3.6.2-carto.4

**Release date**: 2018-03-14

Changes:
 - Update mapnik to [v3.0.15.6](https://github.com/CartoDB/mapnik/blob/v3.0.15-carto/CHANGELOG.carto.md#30156), which includes fixes and improvements for raster markers cache.

## 3.6.2-carto.3

**Release date**: 2018-02-26

Changes:
 - Build also for Node9. [Upstream](https://github.com/mapnik/node-mapnik/commit/690f351c20cb5d08e57df5033d70d13417625da7)
 - Update Mapnik to v3.0.15.4


## 3.6.2-carto.2

**Release date**: 2018-01-26

Changes:
 - Make sure the submodules are included [#11](https://github.com/CartoDB/node-mapnik/pull/11)
 - Change harfbuzz dep to v1.3.0 [#12](https://github.com/CartoDB/node-mapnik/pull/12)


## 3.6.2-carto.1

**Release date**: 2018-01-05

Changes:
 - Add this very changelog
 - Pass variables into grid_renderer [#6](https://github.com/CartoDB/node-mapnik/pull/6)
 - Upgrade mapnik to v3.0.15.2 and mapnik-vector-tile to v1.5.0 [#9](https://github.com/CartoDB/node-mapnik/pull/9)
 - Pass variables to replace tokens in queries for MVT rendering [#809](https://github.com/mapnik/node-mapnik/pull/809) backported to [#10](https://github.com/CartoDB/node-mapnik/pull/10)
 - Add mapnik metrics [#8](https://github.com/CartoDB/node-mapnik/pull/8)


## 3.6.2-carto.0

**Release date**: 2017-12-22

Changes:
 - Use our mason and mapnik versions. See the changes [here](https://github.com/CartoDB/node-mapnik/compare/v3.6.2...v3.6.2-carto.0?expand=1)


## 3.6.2

**Release date**: 2017-06-19

Vanilla node-mapnik v3.6.2 in which the rest of the code is based upon. See https://github.com/mapnik/node-mapnik/blob/v3.6.2/CHANGELOG.md
