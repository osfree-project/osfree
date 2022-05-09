pipeline {
    agent any
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
                    dir '/root/osfree'
                    // args '-v /var/lib/jenkins/workspace/osfree_master@2:/root/osfree'
                }
            }
            steps {
                sh './_wcc.sh'
            }
        }
    }
}
