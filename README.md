# a http dump tool

## build

```sh
mkdir build
cd build
cmake ..
make
```

## run example

start dump http:
```sh
cd examples/protobuf_plugin/
cp ../../httpdump ./
./httpdump eth0 tcp port 80
```

then run:
```sh
python ./send_request.py [url]
```
send a http request which body format is protobuf to [url].
httpdump will capture it,and print protobuf body in text format.


