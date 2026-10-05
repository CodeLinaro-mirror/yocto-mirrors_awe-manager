# Usage: AWE-Manager Shell

Regardless whether [AWE-Manager shell][shell-addon-overview] is used as standalone executable or via the awemgr_client / server architecture, the syntax of the interactive debug shell is the same.

This section tries to give some tips for using the shell.

## Using AWE-Manager Client

- Have your system running either [AWE-Manager Service][awe-manager-service-overview] or have the [Shell Addon][shell-addon-overview] integrated in your system control service using AWE-Manager library.
    - you will have the debug shell available at a socket port, typically 15100.
- Start the awemgr-client to connect to your target by
    - cloning https://bitbucket.org/dspconcepts/awe-manager
    - running `uv run --with pyyaml --with prompt_toolkit awe_manager/apps/awemgr_client/awemgr_client.py` from within the workspace, use parameters which fit to your setup. Try `-h` to obtain a list of possible parameters.
    - you will be greeted with the shell prompt `awemgr-shell > `

## Communication Tracing

This allows to intercept and trace the communication of {{name.awe_mgr}} with the audio processing AWE Core instances, see [Context View][context-view]. It is particularly useful when the [Shell Addon][shell-addon-overview] is used in a running system service, like [AWE-Manager Service][awe-manager-service-overview].

There are 2 possibilities for tracing, which also can be combined:

- enable trace or debug logs - then print messages will be delivered to the system's trace backend, like syslog or journalctl (backend depends on where {{name.awe_mgr}} is integrated)
- use file storage - this stores the command data sent to AWE-Core and the responses in separate files on the target's files system

### Enabling And Disabling Tracing

Inside the shell, use:

- `comm-trace -on` - to enable only debugging output on the service
- `comm-trace -on -file /tmp/comm-trace.awb` - this will log all communication in files `/tmp/comm-trace.awb.tx` for commands sent to AWE-Core and `/tmp/comm-trace.awb.rx` for their responses.
- `comm-trace -off` - to turn tracing off again. IMPORTANT: this will also finally fclose() the trace files. Before this call the file handles will be maintained open. In other words, the target operating system may still hold data in a cache and only the off-command will flush the data.

### Analysis Of Tracefiles

You can use a [Python AWB parser](https://pypi.org/project/pyawe_awb/) tool. This may already be installed with AWE-TC, but can be used separately as well.

To analyze the dump file with this script, the file needs to contain an EOF marker, consisting of 0-bytes.

- on WSL/Linux, simply add 0-bytes with `truncate -s +8 /tmp/comm-trace.awb.tx`
- then analyze the content with `uvx --from pyawe-awb awb-file /tmp/comm-trace.awb.tx`
