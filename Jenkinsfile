pipeline {
    agent any
    stages {
        agent {
            node {
                label 'linux-docker-i386'
            }
        }
        stage('linux build') {
            steps {
                echo "Running build ${env.BUILD_ID} on ${env.JENKINS_URL}"
                sh './_wcc.sh'
            }
        }
    }
    stages {
        agent {
            node {
                label 'win32-vbox-i386'
            }
        }
        stage('win32 build') {
            steps {
                echo "Running build ${env.BUILD_ID} on ${env.JENKINS_URL}"
                cmd '_wcc.cmd'
            }
        }
    }
    stages {
        agent {
            node {
                label 'os2-vbox-i386'
            }
        }
        stage('win32 build') {
            steps {
                echo "Running build ${env.BUILD_ID} on ${env.JENKINS_URL}"
                cmd '_wcc.cmd'
            }
        }
    }
}
