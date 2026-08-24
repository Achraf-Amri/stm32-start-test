pipeline {
    agent any
    environment {
        DOCKER_IMAGE = 'stm32-build-env'
    }
    stages {
        stage('Clone') {
            steps {
                git url: 'https://github.com/Achraf-Amri/stm32-start-test.git', branch: 'main'
            }
        }
        stage('Build Docker Image') {
            steps {
                sh 'docker build -t $DOCKER_IMAGE .'
            }
        }
        stage('Static Analysis') {
            steps {
                sh '''docker run --rm -u $(id -u):$(id -g) -v $(pwd):/project $DOCKER_IMAGE sh -c "cppcheck --enable=all --error-exitcode=1 --suppress=missingInclude -DSTM32F407xx -D__GNUC__ -ICore/Inc -isystem Drivers/STM32F4xx_HAL_Driver/Inc -isystem Drivers/CMSIS/Device/ST/STM32F4xx/Include -isystem Drivers/CMSIS/Include Core/Src/main.c"'''
            }
        }
        stage('Unit Tests') {
            steps {
                sh 'docker run --rm -u $(id -u):$(id -g) -v $(pwd):/project $DOCKER_IMAGE sh -c "ceedling test:all"'
            }
        }
        stage('Compile Firmware') {
            steps {
                sh 'docker run --rm -u $(id -u):$(id -g) -v $(pwd):/project $DOCKER_IMAGE sh -c "make clean && make"'
            }
        }
        stage('Wireless Flash (ESP32)') {
    steps {
        sh 'python3 esp32_ota/scripts/send_firmware.py build/Start_test.bin http://192.168.0.145/flash'
    }
}

       
        stage('Version Artifacts') {
            steps {
                script {
                    env.GIT_SHORT_COMMIT = sh(script: "git rev-parse --short HEAD", returnStdout: true).trim()
                }
                sh '''
                    cp build/Start_test.elf build/firmware_${GIT_SHORT_COMMIT}.elf
                    cp build/Start_test.hex build/firmware_${GIT_SHORT_COMMIT}.hex
                    cp build/Start_test.bin build/firmware_${GIT_SHORT_COMMIT}.bin
                '''
            }
        }
        stage('Archive Artifacts') {
            steps {
                archiveArtifacts artifacts: 'build/firmware_*.elf, build/firmware_*.hex, build/firmware_*.bin', fingerprint: true
            }
        }
    }

post {
        success {
            mail to: 'achraf.amri@enicar.ucar.tn',
                 subject: "Build ${env.BUILD_NUMBER} - SUCCES",
                 body: "Le firmware a ete compile, teste et versionne avec succes.\nVoir : ${env.BUILD_URL}"
            slackSend(channel: '#new-channel', color: 'good', message: "Build ${env.BUILD_NUMBER} reussi - firmware pret : ${env.BUILD_URL}")
        }
        failure {
            mail to: 'achraf.amri@enicar.ucar.tn',
                 subject: "Build ${env.BUILD_NUMBER} - ECHEC",
                 body: "Le pipeline a echoue. Voir les logs : ${env.BUILD_URL}console"
            slackSend(channel: '#new-channel', color: 'danger', message: "Build ${env.BUILD_NUMBER} echoue - voir ${env.BUILD_URL}console")
        }
    }
}
