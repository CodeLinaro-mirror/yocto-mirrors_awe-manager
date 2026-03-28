// ****************************************************************************
//
// Build scripts for Windows
//
// ****************************************************************************

pipeline {
    agent {
        label 'windows-lab-agent'
    }
    environment {
        CODECOV_TOKEN = credentials('dspc-prod-jenkins/codecov')
        HATCH_INDEX_REPO = "https://artifactory.ops.dspconcepts.com/artifactory/api/pypi/awepy-dspc-pypi-local"
    }
    stages {
        stage('Environment Info') {
            steps {
                bat 'echo ===== WINDOWS AGENT VERIFICATION ====='
                bat 'echo Agent name: %COMPUTERNAME%'
                bat 'echo Jenkins workspace: %WORKSPACE%'
                bat 'echo Running as user: %USERNAME%'
                bat 'systeminfo | findstr /B /C:"OS Name" /C:"OS Version" /C:"System Type"'
            }
        }
        stage('Build Windows Distribution') {
            steps {
                powershell '''
                    cmake --preset windows-distribution
                    cmake --build --preset windows-distribution --config Release --target package
                '''
                dir('build-Windows/windows-distribution') {
                    archiveArtifacts artifacts: '*.zip', fingerprint: true
                }
            }
        }
    }

    post {
        always {
            bat 'echo Build completed at %TIME% on %DATE%'
        }
        // success {
        //     bat 'echo ===== WINDOWS AGENT TEST PASSED ====='
        // }
        // failure {
        //     bat 'echo ===== WINDOWS AGENT TEST FAILED ====='
        // }
    }
}

