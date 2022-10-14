FROM alpine:latest as builder
WORKDIR /root

RUN apk add --no-cache git make build-base && \
    git clone --branch main --single-branch https://github.com/herokukms/alpinevms.git && \
    cd alpinevms && \
    pwd && ls -l && \
    VERBOSE=1 CC=gcc CFLAGS="-static -DFULL_INTERNAL_DATA" LDFLAGS="-static "  GETIFADDRS=musl DNS_PARSER=internal make && \
    ls -l && pwd 

FROM alpine:latest
COPY --from=builder /root/alpinevms/bin/vlmcsd /usr/bin/vlmcsd
COPY --from=builder /root/alpinevms/init_authorized_keys /bin/
WORKDIR /root/

# supply your pub key via `--build-arg ssh_pub_key="$(cat ~/.ssh/id_rsa.pub)"` when running `docker build`
RUN apk add --no-cache openrc openssh &&  \
    ssh-keygen -A \
    && echo -e "PasswordAuthentication no" >> /etc/ssh/sshd_config && \
    sed -i 's/#Port 22/Port 2222/g' /etc/ssh/sshd_config && \
    mkdir -p /root/.ssh && \
    chmod 0700 /root/.ssh && \
    chmod u+x /bin/init_authorized_keys && \
    cd / ln -svf

VOLUME ["/storage", "/sys/fs/cgroup" ]
EXPOSE 1688/tcp
EXPOSE 2222/tcp
#CMD ["/bin/init_authorized_keys"]
ENTRYPOINT ["sh", "-c", "mkdir -p /run/openrc/ ; touch /run/openrc/softlevel; rc-status; rc-service sshd start; /bin/init_authorized_keys; /usr/bin/vlmcsd -D -d -e -H 20348 -C 1036 -v"]