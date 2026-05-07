# Test Plan

This page describes the test plan on how to ensure a correct working AWE-Manager component.

## Testing Scopes

Testing AWE-Manager foresees several levels of testing:

- system / manual tests
- integration tests
- unit tests

For a list of test cases, please refer to the [Traceability Report][traceability-report].

**System/manual tests** are not defined in the scope of this document. They will typically be performed once the complete AWE software has been integrated and the system is up and running. Likely, they will actually be even implemented by a customer as part of "customer test" applications. This could be an application testing a volume is correctly set with a test design running and a tester observing the audio volume change.

These tests are conducted only on the target platform.

**Integration tests** are tests where no application will be required and the functioning of AWE-Manager is tested with "test programs" provided by DSPC. For example, command line utilities are used to configure AWE-Manager, load and execute designs and control them. The command line test programs would not require a functioning AWE-Controller service.

Integration tests are designed to also execute on PC platforms or on build server.

**Unit Tests** do test single functions of a component and do not require other components
to be up and running during test execution.

Unit tests are typically implemented for the sub-components, testing the specific component in the gtest/gmock testing framework.

Unit tests are also available on PC. Whenever required those tests will use mocking of other components.

