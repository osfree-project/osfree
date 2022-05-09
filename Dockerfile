ARG BASE_IMAGE=amd64/debian:bullseye

FROM $BASE_IMAGE

COPY ./_setup.sh /root

RUN apt update -y && \
  apt install -y openjdk-11-jre-headless openssh-server

RUN /root/_setup.sh

ENTRYPOINT [ "/usr/sbin/sshd" ]
