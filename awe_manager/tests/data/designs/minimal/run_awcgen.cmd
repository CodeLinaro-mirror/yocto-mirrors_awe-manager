:<<BATCH
    @echo off
    set TARGET_FILE_DIR=./target_files
    set AWB_FILE=%TARGET_FILE_DIR%/scaler_ctrl_only.awb
    set CTRLH_FILE=%TARGET_FILE_DIR%/scaler_ctrl_only_ControlInterface.h
    set AWC_GENCFG=../../../../components/awe_awc/config/gencfg_awc/gen_cfg.yml

    awe-awcgen -o %TARGET_FILE_DIR% --gen_spec %AWC_GENCFG% --awb %AWB_FILE% --ctrl_h %CTRLH_FILE% -l INFO
    exit /b
BATCH

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
TARGET_FILE_DIR=$SCRIPT_DIR/target_files
AWB_FILE=$TARGET_FILE_DIR/scaler_ctrl_only.awb
CTRLH_FILE=$TARGET_FILE_DIR/scaler_ctrl_only_ControlInterface.h
AWC_GENCFG=$SCRIPT_DIR/../../../../components/awe_awc/config/gencfg_awc/gen_cfg.yml

awe-awcgen -o $TARGET_FILE_DIR --gen_spec $AWC_GENCFG --awb $AWB_FILE --ctrl_h $CTRLH_FILE -l INFO
