from ubuntu:jammy

RUN apt update -y && apt install -y curl python3 python3-distutils unzip
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/local/bin sh
RUN curl -fsSL https://bootstrap.pypa.io/get-pip.py | python3

# install uf2conv
RUN curl -L https://github.com/microsoft/uf2/archive/ffe793f973ff8fc288365a2183c9727aa40aed07.zip -o /root/uf2.zip && \
  unzip /root/uf2.zip -d /root/ && \
  mv /root/uf2-ffe793f973ff8fc288365a2183c9727aa40aed07/ /root/uf2

COPY ./install.sh ./
RUN bash install.sh