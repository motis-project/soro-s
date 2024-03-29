FROM alpine:3.14
ARG TARGETARCH
ADD soro-s-linux-$TARGETARCH/soro-s-linux-$TARGETARCH.tar.bz2 /
RUN addgroup -S soro-s && adduser -S soro-s -G soro-s && \
    chown -R soro-s:soro-s /soro-s/

EXPOSE 8080
VOLUME ["/resources"]
WORKDIR /soro-s
USER soro-s
CMD ["/soro-s/soro-server", "--resource_dir", "/resources"]
