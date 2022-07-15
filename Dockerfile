FROM alpine:latest as builder
WORKDIR /root
RUN apk add --no-cache git make build-base && \
    git clone --branch main --single-branch https://github.com/herokukms/alpinevms.git && \
    cd alpinevms && \
    pwd && ls -l && \
    VERBOSE=1 CC=gcc CFLAGS="-static -DFULL_INTERNAL_DATA" LDFLAGS="-static "  GETIFADDRS=musl DNS_PARSER=internal make && \
    ls -l && pwd

FROM alpine:latest
WORKDIR /root/
COPY --from=builder /root/alpinevms/bin/vlmcsd /usr/bin/vlmcsd
EXPOSE 1688/tcp
CMD [ "/usr/bin/vlmcsd", "-D", "-d", "-e", "-H 20348", "-C 1036" ]
