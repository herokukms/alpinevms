# syntax = docker/dockerfile:1.4.0
FROM alpine:latest as builder
WORKDIR /root

RUN apk add --no-cache git make build-base libcurl curl-dev openssl-dev && \
    git clone https://github.com/herokukms/alpinevms.git && \
    cd alpinevms && \
    mkdir -p bin && \
    cat startup > /root/alpinevms/bin/startup && \
    pwd && ls -l && \ 
    VERBOSE=1 CC=gcc CFLAGS="$CFLAGS -static -DFULL_INTERNAL_DATA -D_GNU_SOURCE" LDFLAGS="-static /usr/lib/libpthread.a"  GETIFADDRS=musl DNS_PARSER=internal make -j1 && \
    ls -l && pwd 

FROM alpine:latest
COPY --from=builder /root/alpinevms/bin/vlmcsd /usr/bin/vlmcsd
COPY --from=builder /root/alpinevms/bin/vlmcs /usr/bin/vlmcs
COPY --from=builder /root/alpinevms/bin/startup /usr/bin/startup
WORKDIR /root/

# supply your pub key via `--build-arg ssh_pub_key="$(cat ~/.ssh/id_rsa.pub)"` when running `docker build`

RUN apk add --no-cache openrc openssh libcurl &&  \
    mkdir -p /root/.ssh && \
    chmod 0700 /root/.ssh && \
    ssh-keygen -A \
    && echo -e "PasswordAuthentication no" >> /etc/ssh/sshd_config && \
    sed -i 's/#Port 22/Port 2222/g' /etc/ssh/sshd_config && \
    chmod 0755 /usr/bin/startup && \
    cd / ln -svf

VOLUME ["/storage", "/sys/fs/cgroup" ]
EXPOSE 1688/tcp
EXPOSE 2222/tcp
ENTRYPOINT ["/usr/bin/startup"]