pipeline {
    agent any
    environment {
        ARCH = 'i386'
    }
    stages {
        //stage('builds on github') {
        //    steps {
        //        script {
        //            final String url = "https://api.github.com/repos/osfree-project/osfree/dispatches"
        //            final String response = sh(script: "curl --request POST --url '$url' \
        //                --header 'authorization: Bearer ${env.GITHUB_TOKEN}' \
        //                --data '{"event_type": "hello"}'", returnStdout: true).trim()
        //
        //            echo response
        //        }
        //    }
        //}
        //stage('vbox builds') {
        //    steps {
        //        ////
        //    }
        //}
        stage('docker builds') {
            agent {
                //docker {
                //    label 'local-docker-i386'
                //    image "${ARCH}/osfreebld:latest"
                //    args "-v ${WORKSPACE}:/root/osfree -w /root/osfree --network bridge"
                //    reuseNode true
                //}
                dockerfile {
                    label 'local-docker-i386'
                    filename 'Dockerfile'
                    additionalBuildArgs "--build-arg BASE_IMAGE=${env.ARCH}/debian:bullseye -t ${ARCH}/osfreebld:latest"
                    args "-v ${WORKSPACE}:/root/osfree -w /root/osfree \
                          -v /var/lib/jenkins:/root --network bridge"
                    reuseNode true
                }
            }
            steps {
                echo "Downloading the agent"
                sh 'wget ${JENKINS_URL}jnlpJars/agent.jar -P /root'

                echo "Starting JNLP agent in container"
                sh 'java -jar /root/agent.jar \
                    -jnlpUrl ${JENKINS_URL}computer/${NODE_NAME}/jenkins-agent.jnlp \
                    -secret ${JNLP_SECRET} -workDir /var/lib/jenkins'

                echo "Running build ${env.BUILD_ID} on ${env.JENKINS_URL}"
                sh './_wcc.sh'
            }
        }
    }
}
