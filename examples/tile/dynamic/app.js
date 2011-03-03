#!/usr/bin/env node

var mapnik = require('mapnik')
  , mercator = require('mapnik/sphericalmercator')
  , http = require('http')
  , url = require('url')
  , tile = 256
  , img = 'google_point_8.png'
  , path = require('path')
  , async_render = true;

var usage = 'usage: app.js <port>';

var port = process.ARGV[2];

if (!port) {
   console.log(usage);
   process.exit(1);
}

// postgis table
var table = 'points9';

//var merc = '+proj=merc +lon_0=0 +lat_ts=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs +over';
var merc = '+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over';

// here we build up map object using an XML string
// in time I will expose the mapnik api to do be able
// to do this in pure javascript...

// map
var s = '<Map buffer-size="128" srs="' + merc + '">';

// style
s += '<Style name="style">';
s += '<Rule>';
s += '<PointSymbolizer file="point.png" allow-overlap="true" />';
//s += '<PointSymbolizer file="point.svg" allow-overlap="true" />';
//s += '<MarkersSymbolizer type="ellipse" fill="red" allow-overlap="true" placement="point"/>';
s += '</Rule>';
s += '</Style>';

// layer
s += '<Layer name="world" srs="' + merc + '">';
s += '<StyleName>style</StyleName>';

// postgis datasource
var ds = '<Datasource>';
ds += '<Parameter name="dbname">tiledb</Parameter>';
ds += '<Parameter name="extent">-20005048.4188,-9039211.13765,19907487.2779,17096598.5401</Parameter>';
ds += '<Parameter name="geometry_field">the_geom</Parameter>';
ds += '<Parameter name="srid">900913</Parameter>';
ds += '<Parameter name="type">postgis</Parameter>';
ds += '<Parameter name="user">postgres</Parameter>';
ds += '<Parameter name="initial_size">1</Parameter>';
ds += '<Parameter name="max_size">1</Parameter>';
ds += '<Parameter name="table">';
ds += table;
ds += '</Parameter>';
ds += '</Datasource>';

s += ds + '</Layer>';
s += '</Map>';


http.createServer(function(req, res) {

  var query = url.parse(req.url, true).query;

  if (query &&
      query.x !== undefined &&
      query.y !== undefined &&
      query.z !== undefined
      ) {

      var map = new mapnik.Map(tile, tile);

      res.writeHead(200, {'Content-Type': 'image/png'});

      var bbox = mercator.xyz_to_envelope(parseInt(query.x), parseInt(query.y), parseInt(query.z), false);

      map.from_string(s, path.join(__dirname, './'));
      //map.load('../../examples/stylesheet.xml')
      //console.log(map.to_string());

      if (async_render) {
          map.render(bbox, 'png', function(err, image) {
              if (err) {
                  res.writeHead(500, {
                    'Content-Type': 'text/plain'
                  });
                  res.end(err.message);
              } else {
                  res.end(image);
              }
          });
      }
      else {
          map.zoom_to_box(bbox);
          res.end(map.render_to_string('png'));
      }

  } else {
      res.writeHead(200, {
        'Content-Type': 'text/plain'
      });
      res.end('no x,y,z provided');
  }
}).listen(port);


console.log('Test server listening on port %d', port);
