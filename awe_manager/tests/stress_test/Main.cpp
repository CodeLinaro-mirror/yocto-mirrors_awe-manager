#include "stressTest.h"
#include "testConfig.h"

int main(int argc, char *argv[])
{
    testConfig config;
    if (!config.extractConfig(argc, argv))
    {
        config.showUsage(argv[0]);
        exit(0);
    }

    stressTest test(config);
    test.runTests();
    test.printTestResults();
    exit(0);
}
