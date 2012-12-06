# run all examples just to ensure they don't throw any errors
node ./examples/simple/render.js ./examples/stylesheet.xml /tmp/map.png
node ./examples/simple/blank.js & PID=$!; sleep 2; kill $PID
node ./examples/simple/simple.js & PID=$!; sleep 2; kill $PID
node ./examples/simple/simple_express.js & PID=$!; sleep 2; kill $PID
node ./examples/memory_datasource/simple.js; open memory_points.png
node ./examples/wms/wms_pool.js ./examples/stylesheet.xml 8000 & PID=$!; sleep 2; kill $PID
node ./examples/tile/pool/app.js ./examples/stylesheet.xml 8000 & PID=$!; sleep 2; kill $PID
node ./examples/tile/database/app.js ./examples/stylesheet.xml 8000 & PID=$!; sleep 2; kill $PID
node ./examples/tile/elastic/app.js 8000 & PID=$!; sleep 2; kill $PID

# cleanup
sleep 2;
rm memory_points*
