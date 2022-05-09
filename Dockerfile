FROM $ARCH/debian:bullseye

COPY ./_setup.sh /root

RUN apt update -y && \
  apt install -y openjdk-11-jre

RUN /root/_setup.sh

ENTRYPOINT []
