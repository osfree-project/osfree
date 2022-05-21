@echo off

set WORKSPACE=\src\osfree\tmp\os2
set NODE_NAME=os2-vbox-i386
set JENKINS_URL=http://10.8.10.18:8080/
set JNLP_SECRET=858b61416279941c8658a8d6302400a142460c31d33c0bce6a38100f7f6d7ff9

f:
if exist %WORKSPACE%\slave.jar goto later
echo "Downloading the agent"
wget %JENKINS_URL%jnlpJars/slave.jar -O %WORKSPACE%\slave.jar
:later

cd %WORKSPACE%

:run
echo "Starting JNLP agent"
java -jar %WORKSPACE%\slave.jar -jnlpUrl %JENKINS_URL%computer/%NODE_NAME%/slave-agent.jnlp -secret %JNLP_SECRET%
sleep 10
goto run
