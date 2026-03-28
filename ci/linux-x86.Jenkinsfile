// ****************************************************************************
//
// Build scripts for Linux-x86 toolchain
//
// ****************************************************************************

def checkMisraCompliance(filePath) {
    echo "Analyzing MISRA compliance report at ${filePath}"
    def pattern = '^<tr><td>Rule \\d+\\.\\d+<\\/td><td>Mandatory<\\/td><td>.+(?<!Compliant)<\\/td><\\/tr>$'
    def fileContent = readFile(filePath)
    def lines = fileContent.split("\n")
    for (line in lines) {
        if (line =~ pattern) {
            unstable 'Detected a violation of a mandatory MISRA.'
        }
    }
    echo 'Code is compliant with all mandatory MISRA rules.'
}

pipeline {
    agent none
    environment {
        CODECOV_TOKEN = credentials('dspc-prod-jenkins/codecov')
        HATCH_INDEX_REPO = "https://python.ops.dspconcepts.com/root/awepy/+simple/"
    }
    stages {
        stage('Build and Test') {
            parallel {
                stage('Unit Testing') {
                    agent {
                        kubernetes {
                            inheritFrom 'default-agent'
                            defaultContainer 'builder'
                            yaml '''
                            spec:
                              containers:
                                - name: builder
                                  image: oci.ops.dspconcepts.com/builder/coverage-lcov:latest
                                  ttyEnabled: true
                                  command: [ "tail", "-f", "/dev/null" ]
                                  securityContext:
                                    runAsUser: 1000
                                    runAsGroup: 1000
                                    fsGroup: 1000

                                - name: codecov
                                  image: oci.ops.dspconcepts.com/tools/codecov:latest-jenkins
                                  ttyEnabled: true
                                  command: [ "tail", "-f", "/dev/null" ]
                                  securityContext:
                                    runAsUser: 1000
                                    runAsGroup: 1000
                                    fsGroup: 1000
                            '''
                        }
                    }
                    stages {
                        stage('Build Unit Tests') {
                            steps {
                                sh 'cmake --preset testing'
                                sh 'cmake --build --preset testing --parallel $(nproc)'
                            }
                        }
                        stage('Run Unit Tests') {
                            steps {
                                sh 'cmake --build --preset testing --target coverage'
                            }
                            post {
                                always {
                                    sh "cmake --install ${env.WORKSPACE}/build/testing --prefix 'reports' --component Reports"

                                    // include test results on Jenkins job page
                                    junit allowEmptyResults: true, testResults: 'reports/junit/*.xml'
                                    archiveArtifacts artifacts: "reports/**", allowEmptyArchive: true
                                    container("codecov") {
                                        // upload the report to codecov server
                                        sh """
                                        codecov \
                                            --verbose \
                                            --auto-load-params-from Jenkins \
                                            --url https://codecov.ops.dspconcepts.com \
                                            upload-process \
                                            --fail-on-error \
                                            -n '${env.CHANGE_BRANCH ?: env.GIT_BRANCH}' \
                                            --sha ${GIT_COMMIT} \
                                            --slug dspconcepts/qc-audiolite-integration-awemanager \
                                            --git-service bitbucket \
                                            --file ${env.WORKSPACE}/reports/coverage/awe_manager-coverage.info
                                        """
                                    }

                                    sh 'cmake --build --preset testing --target coverage-html'
                                    dir('html') {
                                        sh "cmake --install ${env.WORKSPACE}/build/testing --prefix . --component HtmlReports"
                                        publishHTML(target: [
                                            reportName: 'Unit Test Coverage Report',
                                            reportDir: 'coverage/awe_manager',
                                            reportFiles: 'index.html',
                                            allowMissing: true,
                                            keepAll: true,
                                        ])
                                    }
                                }
                            }
                        }
                    }
                }
                stage('Cyclomatic Complexity') {
                    agent {
                        kubernetes {
                            inheritFrom 'default-agent'
                            defaultContainer 'lizard'
                            yaml '''
                            spec:
                              containers:
                                - name: lizard
                                  image: oci.ops.dspconcepts.com/tools/lizard:1.17.10
                                  ttyEnabled: true
                                  entrypoint: [ "/bin/bash" ]
                                  command: [ "tail", "-f", "/dev/null" ]
                                  securityContext:
                                    runAsUser: 1000
                                    runAsGroup: 1000
                                    fsGroup: 1000
                            '''
                        }
                    }
                    steps {
                        sh """
                        ${env.WORKSPACE}/scripts/calculate_cyclomatic_complexity.sh \
                                -s ${env.WORKSPACE}/awe_manager \
                                -o "${env.WORKSPACE}/reports/cyclomatic_complexity" \
                                -n "awemanager-cyclomatic-complexity"
                        """
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: "reports/cyclomatic_complexity/**", allowEmptyArchive: true
                            publishHTML(target: [
                                reportName: 'Cyclomatic Complexity Report',
                                reportDir: 'reports/cyclomatic_complexity/',
                                reportFiles: 'awemanager-cyclomatic-complexity.html',
                                allowMissing: true,
                                keepAll: true,
                            ])
                        }
                    }
                }
                stage('Distribution - Linux x86_64') {
                    agent {
                        kubernetes {
                            inheritFrom 'default-agent'
                            defaultContainer 'builder'
                            yaml '''
                            spec:
                              containers:
                                - name: builder
                                  image: oci.ops.dspconcepts.com/builder/linux-x86:gcc-14
                                  ttyEnabled: true
                                  command: [ "tail", "-f", "/dev/null" ]
                                  securityContext:
                                    runAsUser: 1000
                                    runAsGroup: 1000
                                    fsGroup: 1000
                                  volumeMounts:
                                    - mountPath: "/etc/cppcheckpremium"
                                      name: cppcheckpremium-license
                                      readOnly: true
                              volumes:
                                - name: cppcheckpremium-license
                                  secret:
                                    secretName: cppcheck-lic
                            '''
                        }

                    }
                    environment {
                        // Add cppcheck tool path. Not sure if there's a better way to do this...
                        CPPCHECK_ROOT = "/home/jenkins/tools/io.jenkins.plugins.generic_tool.GenericToolInstallation/cppcheck/cppcheckpremium-25.3.0.2/"
                        PATH = "${env.CPPCHECK_ROOT}:/home/jenkins/.local/bin:${env.PATH}"
                    }
                    tools {
                        generic 'cppcheck'
                    }
                    stages {
                        stage('Build Distribution Package') {
                            steps {
                                // create Linux-X64 release executable
                                sh 'cmake --preset distribution'
                                sh 'cmake --build --preset distribution --parallel $(nproc) --target package package_source'
                                dir('build/distribution') {
                                    archiveArtifacts artifacts: '*.tar.bz2', fingerprint: true
                                }
                            }
                        }
                        stage('Static Analysis') {
                            steps {
                                sh 'cmake --build --preset distribution --target cppcheck'
                                sh "cmake --install ${env.WORKSPACE}/build/distribution --prefix ${env.WORKSPACE}/reports --component Reports"
                                archiveArtifacts artifacts: 'reports/**/*.xml', allowEmptyArchive: true

                                // This is a dep for cppcheck-htmlreport. Ideally it would get installed
                                // with the tool, but that isn't how it's set up right now.
                                sh '/usr/bin/env python3 -m pip install pygments'

                                sh 'cmake --build --preset distribution --target cppcheck-html'
                                catchError(buildResult: 'SUCCESS', stageResult: 'UNSTABLE') {
                                    sh 'cmake --build --preset distribution --target misra'
                                }
                                sh "cmake --install ${env.WORKSPACE}/build/distribution --prefix ${env.WORKSPACE}/html --component HtmlReports"
                                publishHTML(target: [
                                    reportName: 'Compliance Reports',
                                    reportDir: 'html',
                                    reportFiles: 'misra/awe_manager.html,cppcheck/awe_manager/index.html',
                                    reportTitles: 'MISRA Compliance,Detailed Analysis',
                                    allowMissing: true,
                                    keepAll: true,
                                ])
                                script { checkMisraCompliance("${env.WORKSPACE}/html/misra/awe_manager.html") }
                            }
                        }
                    }
                }
                stage('Documentation') {
                    agent {
                        kubernetes {
                            inheritFrom 'default-agent'
                            defaultContainer 'mkdocs'
                            yaml '''
                            spec:
                              containers:
                                - name: mkdocs
                                  image: oci.ops.dspconcepts.com/tools/mkdocs:latest
                                  ttyEnabled: true
                                  command: [ "tail", "-f", "/dev/null" ]
                                  securityContext:
                                    runAsUser: 1000
                                    runAsGroup: 1000
                                    fsGroup: 1000
                            '''
                        }
                    }
                    environment {
                        PIP_CONFIG_FILE = "${env.WORKSPACE}/pip.conf"
                        ENABLE_PDF_EXPORT = "1"
                    }
                    steps {
                        withCredentials([
                            usernameColonPassword(
                                credentialsId: 'dspc-prod-jenkins/devpi',
                                variable: 'PIP_INDEX_CREDS',
                            )
                        ]) {
                            script {
                                def git_rev = sh(returnStdout: true, script: 'ci/get_git_revision.sh').trim()
                                env.GIT_VERSION = "${git_rev}"
                                echo "Evaluated GIT label: ${env.GIT_VERSION}"
                            }

                            // get access to DSPC PyPi
                            sh '''
                            umask 077
                            cat << EOF > $PIP_CONFIG_FILE
                            [global]
                            index-url = https://${PIP_INDEX_CREDS}@python.ops.dspconcepts.com/root/awepy/+simple
                            EOF
                            '''.stripIndent()

                            // for later if it will not be possible to get req-tracer directly into Docker container
                            // sh '''python -m pip install req-tracer'''

                            sh 'mkdocs build'

                            publishHTML(target: [
                                reportName: 'Documentation',
                                reportDir: 'dist/www',
                                reportFiles: "index.html,awe-manager-doc-${env.GIT_VERSION}.pdf",
                                reportTitles: 'Documentation,PDF-Doc',
                                allowMissing: true,
                                keepAll: true,
                            ])
                        }
                    }
                }
            }
        }
    }
}
