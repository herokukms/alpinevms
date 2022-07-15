FROM alpine:latest as builder
WORKDIR /root
RUN apk add --no-cache git make build-base && \
    CFLAGS="-static -DFULL_INTERNAL_DATA" make

FROM alpine:latest
WORKDIR /root/
COPY --from=builder /root/vlmcsd/bin/vlmcsd /usr/bin/vlmcsd
EXPOSE 1688/tcp
CMD [ "/usr/bin/vlmcsd", "-D", "-d" ]
