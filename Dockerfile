FROM debian:bullseye AS builder
WORKDIR /app
COPY . /app
COPY ./.sources.list /etc/apt/sources.list
RUN apt-get update && apt-get -y install \
	g++ \
	make \
	cmake \
	git \
	libboost-filesystem-dev \
	libboost-regex-dev \
	libboost-program-options-dev \
	libboost-regex-dev \
	libboost-system-dev \
	gdebi-core \
	dpkg-dev

WORKDIR /build
# TODO find a way to not hardcode this,
# as the version determines the path we need to pass to WORKDIR
RUN apt-get source libpng-dev=1.6.37-3
RUN apt-get -y build-dep libpng-dev=1.6.37-3
WORKDIR libpng1.6-1.6.37
RUN ./configure
RUN make CFLAGS=-static
RUN make install

WORKDIR /build
RUN git clone https://github.com/Samsung/rlottie
WORKDIR rlottie
WORKDIR build
RUN cmake -DBUILD_SHARED_LIBS=OFF ..
RUN make install

WORKDIR /build
RUN git clone https://github.com/signalstickers/apngasm -b fork
WORKDIR apngasm
WORKDIR build
RUN cmake -DBUILD_SHARED_LIBS=off ..
RUN make package
RUN gdebi --non-interactive ./lib/package/libapngasm*.deb

WORKDIR /app
RUN make

FROM debian:latest AS runner
WORKDIR /app
COPY --from=builder /app/tgs2apng .
CMD ["./tgs2apng"]
