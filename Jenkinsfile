pipeline {
    agent any
    environment {
        ARCH = 'i386'
        // WORKSPACE = "/var/lib/jenkins/workspace/osfree_master_${env.EXECUTOR_NUMBER}"
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
                dockerfile {
                    label 'main'
                    filename 'Dockerfile'
                    additionalBuildArgs "--build-arg BASE_IMAGE=${env.ARCH}/debian:bullseye"
                    reuseNode true
                }
            }
            steps {
                echo "Running build ${env.BUILD_ID} on ${env.JENKINS_URL}"

                dir("/var/lib/jenkins/workspace/osfree_master") {
                    sh './_wcc.sh'
                }
            }
        }
    }
}
