FROM i386/debian:bullseye

COPY ./_setup.sh /root

RUN /root/_setup.sh

ENTRYPOINT [ "/bin/sh", "-c", "cd /root/osfree && ./_wcc.sh" ]
