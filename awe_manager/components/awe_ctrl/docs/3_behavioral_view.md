# AWE CTRL: Behavioral View

This section introduces the concepts of the communication of awe_CTRL with {{name.awe_host}}.

## Tuning Commands 

The `awectrl_transact()` as the underlying method to communicate tuning commands is a **SYNCHRONOUS** call. This means that it waits for the other side to respond after it has sent a data package.

The following picture shows a abstracted view of how data is being communicated. This concept is followed for all communication backends.

![file](diagrams/out/3_behavioral.svg)


Data is being written to the backend and the other party is notified about this data.

In the case of a socket this is done by the socket iself - data will just be readable on the other side. The remote party's reading will pick up the data and provide the response. Correspondingly, the sender will wait in it's reading function.

In case of (shared) memory, the other side is notified by an interrupt (IPCC). Similarly, the "other side", e.g. the DSP will signal awe_CTRL component that a response is available. Also in this case the `awectrl_transact()` method will wait until the response has arrived.

!!! note
    Currently, there is no timeout implemented on `awectrl_transact()` yet for all backends. It is foreseen but not fully available at this time.

## Event Data

The `aweevent_read()` method reads event data from the backend. It is designed as a blocking function which waits until an event has been indicated.

![file](diagrams/out/3_behavioral_event.svg)
