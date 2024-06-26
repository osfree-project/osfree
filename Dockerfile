ARG BASE_IMAGE=i386/debian:bullseye

FROM $BASE_IMAGE

COPY ./tools/conf/scripts/_setup.sh /root

RUN apt update -y && \
  apt install -y openjdk-11-jre-headless openssh-server git

RUN /root/_setup.sh

ENTRYPOINT [ "/bin/sh", "/root/osfree/tmp/linux/workspace/osfree-lnx/tools/conf/scripts/jnlp-lnx.sh" ]
