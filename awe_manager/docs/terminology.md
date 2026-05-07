# Terminology

- _AWB_ **--** Audio Weaver instruction file - commands stored in efficient binary form
- _preset-AWB_ **--** same format as _AWB_; only contains commands to modify variables
- _AWC_ **--** Audio Weaver Container - a collection of AWE target files including an "index file"; configuration for {{name.awe_mgr}}

- <a name="awe-controller"></a>_AWE Controller_ **--** application using {{name.awe_mgr}}

- <a name="audio-proc"></a>_Audio Processors_ **--** Any system component including an AWE Core instance; might exist on DSP or on ARM CPU cores
- <a name="awecore"></a>_AWE Core Library_ **--** the DSPC target library executing the _Signal Flow_; being integrated by an _Audio Processors_
- <a name="signalflow"></a>_Signal Flow_ **--** the design document being created in AWE Designer tool on PC

- <a name="awetc"></a>_AWE-TC_ **--** AWE Target Configurator framework to help integrating all of the above mentioned items to embedded target platforms; second stage tooling which can be executed in CM/CI frameworks too (more information available outside of the context of this document)
