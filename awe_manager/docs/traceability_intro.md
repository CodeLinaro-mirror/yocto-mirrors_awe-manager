# Traceability Introduction

The report data is extracted from code and documentation. A DSPC `req-tracer` tool is used to parse the sources to extract information about so called specification objects.

A specification object is a simple form of describing an item, like a _feature_, _requirement_ or _test case_ description. This is the type or category of a specification object.

Specification objects may provide coverage to other specification objects which, in return, may provide coverage to further objects. 
For example, a _test case_ might provide coverage to a _design_ or _requirement_. A _requirement_ specification object might cover
a _high-level requirement_ or _feature_. 

`test case` -- covers --> `requirement` -- covers --> `feature`

Objects may also state that they require coverage by an object of a specific type or category, e.g., a _feature_ is only confirmed if at least one _requirement_ has provided coverage.

The category of a specification object can be freely chosen. The following categories are currently defined and used for the integration of AWE-Manager:

- `feat` - feature: a high-level description of functionality, typically defined by customer or derived by DSPC, like: "there should be runtime
           control of a running AWE design"
- `req` - requirement: a more refined description of a feature a user can experience or specific functionality a component
          requires to fulfill a feature
- `dsn` - design decision: similar to `req` but mostly non functional description; describes how things are implemented; for example: data is shared via shared-memory
- `itest` - integration test - can only function when other (system) components (like AWE-Server) are available
- `utest` - unit test: verifies functioning of a dedicated method or class; needs no other components
