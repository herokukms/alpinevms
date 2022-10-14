FROM alpine:latest as builder
WORKDIR /root

RUN apk add --no-cache git make build-base && \
    git clone --branch main --single-branch https://github.com/herokukms/alpinevms.git && \
    cd alpinevms && \
    pwd && ls -l && \
    VERBOSE=1 CC=gcc CFLAGS="-static -DFULL_INTERNAL_DATA" LDFLAGS="-static "  GETIFADDRS=musl DNS_PARSER=internal make && \
    ls -l && pwd 

FROM alpine:latest
# supply your pub key via `--build-arg ssh_pub_key="$(cat ~/.ssh/id_rsa.pub)"` when running `docker build`
ARG ssh_pub_key
RUN apk add --no-cache openrc openssh &&  \
    ssh-keygen -A \
    && echo -e "PasswordAuthentication no" >> /etc/ssh/sshd_config && \
    mkdir -p /root/.ssh && \
    chmod 0700 /root/.ssh && \
    echo "$ssh_pub_key" > /root/.ssh/authorized_keys
WORKDIR /root/
COPY --from=builder /root/alpinevms/bin/vlmcsd /usr/bin/vlmcsd
EXPOSE 1688/tcp
EXPOSE 22/tcp
#CMD [ "rc-service sshd start ;","/usr/bin/vlmcsd", "-D", "-d", "-e", "-H 20348", "-C 1036", "-v" ]
ENTRYPOINT ["sh", "-c", "mkdir -p /run/openrc/ ; touch /run/openrc/softlevel; rc-status; rc-service sshd start; /usr/bin/vlmcsd -D -d -e -H 20348 -C 1036 -v"]
