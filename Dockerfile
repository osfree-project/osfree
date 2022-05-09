FROM i386/debian:bullseye

COPY ./_setup.sh /root

RUN apt update -y && \
  apt install openjdk-11-jre

RUN /root/_setup.sh

ENTRYPOINT []
