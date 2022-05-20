#! /bin/sh
#

WORKSPACE=/root
JENKINS_URL=http://10.8.10.18:8080/
JNLP_SECRET=

echo "Downloading the agent"
wget ${JENKINS_URL}jnlpJars/agent.jar -O /root/agent.jar

echo "Starting JNLP agent in container"
java -jar /root/agent.jar \
    -jnlpUrl ${JENKINS_URL}computer/${NODE_NAME}/jenkins-agent.jnlp \
    -secret ${env.JNLP_SECRET} -workDir ${WORKSPACE} &"
