@echo off

set WORKSPACE=\src\osfree\tmp\win32
set NODE_NAME=win32-vbox-i386
set JENKINS_URL=http://10.8.10.18:8080/
set JNLP_SECRET=c26e96f71052bcfbd8f8b7d11ed9b9af9b80cea2d14a06261d83fcd3f4e64029

e:
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
