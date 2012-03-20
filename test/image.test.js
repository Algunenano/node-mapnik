var mapnik = require('mapnik');
var fs = require('fs');
var path = require('path');

var Image = mapnik.Image;

exports['test images'] = function(beforeExit, assert) {
    // no 'new' keyword
    assert.throws(function() { Image(1, 1); });

    // invalid args
    assert.throws(function() { new Image(); });
    assert.throws(function() { new Image(1); });
    assert.throws(function() { new Image('foo'); });
    assert.throws(function() { new Image('a', 'b', 'c'); });

    var im = new Image(256, 256);
    assert.ok(im instanceof Image);

    assert.equal(im.width(), 256);
    assert.equal(im.height(), 256);
    var v = im.view(0, 0, 256, 256);
    assert.ok(v instanceof mapnik.ImageView);
    assert.equal(v.width(), 256);
    assert.equal(v.height(), 256);
    assert.equal(im.encodeSync().length, v.encodeSync().length);

    im.save('test/tmp/image.png');

    var im2 = new Image.open('test/tmp/image.png');
    assert.ok(im2 instanceof Image);

    assert.equal(im2.width(), 256);
    assert.equal(im2.height(), 256);

    assert.equal(im.encodeSync().length, im2.encodeSync().length);

    var im_blank = new Image(4, 4);
    assert.equal(im_blank.painted(), false);
    assert.equal(im_blank.background, undefined);

    var m = new mapnik.Map(4, 4);

    m.render(im_blank, {},function(err,im_blank) {
        assert.equal(im_blank.painted(), false);
        assert.equal(im_blank.background, undefined);
    });

    var im_blank2 = new Image(4, 4);
    assert.equal(im_blank2.painted(), false);
    assert.equal(im_blank2.background, undefined);

    var m2 = new mapnik.Map(4, 4);

    m2.background = new mapnik.Color('green');
    m2.render(im_blank2, {},function(err,im_blank2) {
        assert.equal(im_blank2.painted(), false);
        assert.ok(im_blank2.background);
    });


    var im_blank3 = new Image(4, 4);
    assert.equal(im_blank3.painted(), false);
    assert.equal(im_blank3.background, undefined);

    var m3 = new mapnik.Map(4, 4);
    var s = '<Map>';
    s += '<Style name="points">';
    s += ' <Rule>';
    s += '  <PointSymbolizer />';
    s += ' </Rule>';
    s += '</Style>';
    s += '</Map>';
    m3.fromStringSync(s);

    var mem_datasource = new mapnik.MemoryDatasource({'extent': '-180,-90,180,90'});
    mem_datasource.add({ 'x': 0, 'y': 0 });
    mem_datasource.add({ 'x': 1, 'y': 1 });
    mem_datasource.add({ 'x': 2, 'y': 2 });
    mem_datasource.add({ 'x': 3, 'y': 3 });
    var l = new mapnik.Layer('test');
    l.srs = m3.srs;
    l.styles = ['points'];
    l.datasource = mem_datasource;
    m3.add_layer(l);
    m3.zoomAll();
    m3.render(im_blank3, {},function(err,im_blank3) {
        assert.equal(im_blank3.painted(), true);
        assert.equal(im_blank3.background, undefined);
    });

    var gray = new mapnik.Image(256,256);
    gray.background = new mapnik.Color('white');
    gray.setGrayScaleToAlpha();
    var gray_view = gray.view(0,0,gray.width(),gray.height());
    assert.equal(gray_view.isSolid(),true);
    var pixel = gray_view.getPixel(0,0);
    assert.equal(pixel.r,255);
    assert.equal(pixel.g,255);
    assert.equal(pixel.b,255);
    assert.equal(pixel.a,255);

    gray.background = new mapnik.Color('black');
    gray.setGrayScaleToAlpha();
    var pixel2 = gray_view.getPixel(0,0);
    assert.equal(pixel2.r,255);
    assert.equal(pixel2.g,255);
    assert.equal(pixel2.b,255);
    assert.equal(pixel2.a,0);

    gray.setGrayScaleToAlpha(new mapnik.Color('green'));
    var pixel3 = gray_view.getPixel(0,0);
    assert.equal(pixel3.r,0);
    assert.equal(pixel3.g,128);
    assert.equal(pixel3.b,0);
    assert.equal(pixel3.a,255);
};
