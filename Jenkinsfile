// labels for Jenkins node types we will build on
//def labels = ['linux-docker-i386', 'os2-vbox-i386', 'win32-vbox-i386']
//def builders = [:]
//
//for (x in labels) {
//    // Need to bind the label variable before the closure - can't do 'for (label in labels)'
//    def label = x
//
//    // Create a map to pass in to the 'parallel' step so we can fire all the builds at once
//    builders[label] = {
//        node (label) {
//            stage ('Pull submodules') {
//                cmd 'git submodule update --init --recursive'
//                cmd 'git submodule update --remote --recursive'
//            }
//
//            stage ('Build') {
//                echo "Running build ${env.BUILD_ID} on ${env.JENKINS_URL}"
//
//                if (label == 'linux-docker-i386')
//                    cmd './_wcc.sh'
//                else
//                    cmd '_wcc.cmd'
//            }
//
//            stage ('Done') {
//                echo 'Done.'
//            }
//        }
//    }
//}
//
//parallel builders
//

node ('os2-vbox-i386') {
            stage ('Checkout sources') {
                cmd 'git clone https://github.com/osfree-project/osfree'
                cmd 'git submodule update --init --recursive'
                cmd 'git submodule update --remote --recursive'
            }

            stage ('Build') {
                echo "Running build ${env.BUILD_ID} on ${env.JENKINS_URL}"
                cmd '_wcc.cmd'
            }

            stage ('Done') {
                echo 'Done.'
            }
}
