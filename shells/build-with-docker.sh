cp shells/install.sh build-docker/
docker build ./build-docker -t sh-controller-nrf52-builder

BUILD_CMD="bash shells/build.sh"
docker run --rm -v $PWD:/arduino-cli -w /arduino-cli sh-controller-nrf52-builder \
  bash -c "PATH=\$PATH:/root/uf2/utils $BUILD_CMD"