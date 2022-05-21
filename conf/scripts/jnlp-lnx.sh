#! /bin/sh
#

WORKSPACE=/root
JENKINS_URL=http://10.8.10.18:8080/
NODE_NAME=linux-docker-i386
JNLP_SECRET=c215e3e5695b58835883bc6bc6040f8479a7ae5c8d56409f30d2452cd728213c

echo "Downloading the agent"
if [ ! -f "${WORKSPACE}/slave.jar" ]; then
    wget ${JENKINS_URL}jnlpJars/slave.jar -O ${WORKSPACE}/slave.jar
fi

while true; do
    echo "Starting JNLP agent in container"

    java -jar ${WORKSPACE}/slave.jar \
        -jnlpUrl ${JENKINS_URL}computer/${NODE_NAME}/slave-agent.jnlp \
        -secret ${JNLP_SECRET}
    sleep 10
done
