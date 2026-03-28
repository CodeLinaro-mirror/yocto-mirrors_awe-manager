:<<BATCH
    @echo off
    echo "IMPLEMENT ME"
    exit /b
BATCH

for x in dtmf_gen events minimal passthrough set_get; do bash $x/run_awcgen.cmd; done
