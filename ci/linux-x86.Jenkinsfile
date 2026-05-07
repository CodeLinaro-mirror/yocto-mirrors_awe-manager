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
        HATCH_INDEX_REPO = "https://python.ops.dspconcepts.com/root/awepy/+simple/"
    }
    stages {
        stage('Build and Test') {
            parallel {
                stage('Unit Testing') {
                    agent {
                        kubernetes {
                            inheritFrom 'default-lab-agent'
                            defaultContainer 'builder'
                            cloud 'lab-agents'
                            yaml '''
                            spec:
                              nodeSelector:
                                kubernetes.io/os: linux
                              containers:
                                - name: builder
                                  image: oci.ops.dspconcepts.com/builder/coverage-lcov:latest
                                  ttyEnabled: true
                                  command: [ "tail", "-f", "/dev/null" ]
                                  securityContext:
                                    privileged: true
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
                        stage('Run Unit Tests and Collect Coverage') {
                            steps {
                                sh 'cmake --build --preset testing --target coverage'
                            }
                            post {
                                always {
                                    sh "cmake --install ${env.WORKSPACE}/build/Linux/testing --prefix 'reports' --component Reports"

                                    // include test results on Jenkins job page
                                    junit allowEmptyResults: true, testResults: 'reports/junit/*.xml'
                                    archiveArtifacts artifacts: "reports/**", allowEmptyArchive: true

                                    sh 'cmake --build --preset testing --target coverage-html'
                                    sh "cmake --install ${env.WORKSPACE}/build/Linux/testing --prefix 'reports/lcov-html' --component HtmlReports"
                                    dir('reports/lcov-html') {
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
                        CPPCHECK_ROOT = "/home/jenkins/tools/io.jenkins.plugins.generic_tool.GenericToolInstallation/cppcheck/cppcheckpremium-25.8.4/"
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
                                dir('build/Linux/distribution') {
                                    archiveArtifacts artifacts: '*.tar.bz2', fingerprint: true
                                }
                            }
                        }
                        stage('Static Analysis') {
                            steps {
                                sh 'cmake --build --preset distribution --target cppcheck'
                                sh "cmake --install ${env.WORKSPACE}/build/Linux/distribution --prefix ${env.WORKSPACE}/reports --component Reports"
                                archiveArtifacts artifacts: 'reports/**/*.xml', allowEmptyArchive: true

                                // This is a dep for cppcheck-htmlreport. Ideally it would get installed
                                // with the tool, but that isn't how it's set up right now.
                                sh '/usr/bin/env python3 -m pip install pygments'

                                sh 'cmake --build --preset distribution --target cppcheck-html'
                                catchError(buildResult: 'SUCCESS', stageResult: 'UNSTABLE') {
                                    sh 'cmake --build --preset distribution --target misra'
                                }
                                sh "cmake --install ${env.WORKSPACE}/build/Linux/distribution --prefix ${env.WORKSPACE}/reports/static_analysis --component HtmlReports"
                                publishHTML(target: [
                                    reportName: 'Compliance Reports',
                                    reportDir: 'reports/static_analysis',
                                    reportFiles: 'misra/awe_manager.html,cppcheck/awe_manager/index.html',
                                    reportTitles: 'MISRA Compliance,Detailed Analysis',
                                    allowMissing: true,
                                    keepAll: true,
                                ])
                                script { checkMisraCompliance("${env.WORKSPACE}/reports/static_analysis/misra/awe_manager.html") }
                            }
                        }
                    }
                }

                stage('Distribution - Linux AWE-Q') {
                    agent {
                        kubernetes {
                            inheritFrom 'default-agent'
                            defaultContainer 'hgysdk'
                            yaml '''
                            spec:
                              containers:
                                - name: hgysdk
                                  image: oci.ops.dspconcepts.com/builder/snapdragon-auto-hgy-4-1-6-0_hlos_dev_cadence2:r00040-1-sa8255
                                  ttyEnabled: true
                                  command: [ "tail", "-f", "/dev/null" ]
                                  securityContext:
                                    runAsUser: 1000
                                    runAsGroup: 1000
                                    fsGroup: 1000
                                  resources:
                                    limits:
                                      memory: "2Gi"
                                      cpu: "2"
                                    requests:
                                      memory: "1Gi"
                                      cpu: "1"
                            '''
                        }
                    }
                    steps {
                        sh 'cmake --preset aweq-build'
                        sh 'cmake --build --preset aweq-build --parallel $(nproc)'
                        sh "cmake --install build/Linux/aweq-build --prefix ${WORKSPACE}/dist/Linux/aweq"

                        script {
                            def git_rev = sh(returnStdout: true, script: 'ci/get_git_revision.sh').trim()
                            env.GIT_VERSION = "${git_rev}"
                            echo "Evaluated GIT label: ${env.GIT_VERSION}"
                        }
                        zip zipFile: "awe_manager-${env.GIT_VERSION}-Linux-AweQ.zip", dir: 'dist/Linux/aweq', archive:true
                    }
                }

                /* Disable for now - will create branch with fill fix (jhassler)
                stage('Distribution - Windows x86_64') {
                    agent {
                        kubernetes {
                            cloud 'lab-agents'
                            inheritFrom 'windows-lab-agent'
                            defaultContainer 'windows-base'
                            yaml '''
                            spec:
                              containers:
                                - name: windows-base
                                  image: oci.ops.dspconcepts.com/builder/windows-base:ltsc2022
                                  ttyEnabled: true
                                  command: [ "powershell", "-command", "Start-Sleep -Seconds 9999" ]
                            '''
                        }
                    }
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
                */

                stage('Documentation') {
                    agent {
                        kubernetes {
                            inheritFrom 'default-agent'
                            defaultContainer 'mkdocs'
                            yaml '''
                            spec:
                              serviceAccountName: dspc-prod-dochub
                              containers:
                                - name: mkdocs
                                  image: oci.ops.dspconcepts.com/tools/mkdocs:latest
                                  ttyEnabled: true
                                  command: [ "tail", "-f", "/dev/null" ]
                                  securityContext:
                                    runAsUser: 1000
                                    runAsGroup: 1000
                                    fsGroup: 1000
                                - name: aws
                                  image: oci.ops.dspconcepts.com/dockerhub/amazon/aws-cli:latest
                                  command: [ "tail", "-f", "/dev/null" ]
                            '''
                        }
                    }
                    environment {
                        PIP_CONFIG_FILE = "${env.WORKSPACE}/pip.conf"
                        ENABLE_PDF_EXPORT = "1"
                    }
                    stages {
                        stage('Build Documentation') {
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
                                        env.S3_SLOT = (env.TAG_NAME && env.TAG_NAME =~ /^\d+\.\d+\.\d+$/)
                                            ? "awe-manager/${env.TAG_NAME}"
                                            : "awe-manager/dev"
                                    }

                                    // get access to DSPC PyPi
                                    sh '''
                                    umask 077
                                    cat << EOF > $PIP_CONFIG_FILE
                                    [global]
                                    index-url = https://${PIP_INDEX_CREDS}@python.ops.dspconcepts.com/root/awepy/+simple
                                    extra-index-url = https://${PIP_INDEX_CREDS}@python.ops.dspconcepts.com/root/pypi/+simple/
                                    EOF
                                    '''.stripIndent()

                                    // for later if it will not be possible to get req-tracer directly into Docker container
                                    sh '''python -m pip install -r requirements.txt '''

                                    sh "MKDOCS_SITE_URL=https://dochub.dspconcepts.com/${env.S3_SLOT}/ mkdocs build --site-dir dist/documentation"

                                    publishHTML(target: [
                                        reportName: 'Documentation',
                                        reportDir: 'dist/documentation',
                                        reportFiles: "index.html,awe-manager-doc-${env.GIT_VERSION}.pdf",
                                        reportTitles: 'Documentation,PDF-Doc',
                                        allowMissing: true,
                                        keepAll: true,
                                    ])
                                }
                            }
                        }
                        stage('Upload Documentation to Dochub') {
                            steps {
                                container('aws') {
                                    echo "Publishing docs to ${env.S3_SLOT}"
                                    sh "aws s3 sync dist/documentation/ s3://dspc-prod-dochub/${env.S3_SLOT}/ --delete"
                                    // todo: se env.CLOUDFRONT_DISTRIBUTION_ID in ENV section
                                    //catchError(buildResult: 'SUCCESS', stageResult: 'UNSTABLE') {
                                    //    sh "aws cloudfront create-invalidation --distribution-id ${env.CLOUDFRONT_DISTRIBUTION_ID} --paths '/${env.S3_SLOT}/*'"
                                    //}
                                    // ok, that doesnt work either :)
                                    // Josh? :)
                                    // sh 'aws cloudfront list-distributions --query "DistributionList.Items[*].{Id:Id,Origins:Origins.Items[0].DomainName}"'
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
