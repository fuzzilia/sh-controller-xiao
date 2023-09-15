cp shells/install.sh build-docker/
docker build ./build-docker -t sh-controller-nrf52-xiao-builder

BUILD_CMD="bash shells/build.sh"
docker run --rm -v $PWD:/arduino-cli -w /arduino-cli sh-controller-nrf52-xiao-builder \
  bash -c "PATH=\$PATH:/root/uf2/utils:/root/.local/bin/ $BUILD_CMD"