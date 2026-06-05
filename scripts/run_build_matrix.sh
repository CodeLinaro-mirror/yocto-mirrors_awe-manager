#!/usr/bin/env bash
# Build matrix script: runs multiple cmake configure+build+install combinations
# and reports results.

set -euo pipefail

SRCDIR="$(cd "$(dirname "$0")/.." && pwd)"
INSTALL_BASE="/tmp/awemgr_install"
RESULTS_FILE="${SRCDIR}/build/build_matrix_results.txt"

mkdir -p "${SRCDIR}/build"
: > "${RESULTS_FILE}"

log() { echo "$*" | tee -a "${RESULTS_FILE}"; }

run_combo() {
    local name="$1"; shift
    local cmake_args=("$@")

    local build_dir="${SRCDIR}/build/${name}"
    local install_dir="${INSTALL_BASE}/${name}"

    log ""
    log "============================================================"
    log "COMBO: ${name}"
    log "FLAGS: ${cmake_args[*]}"
    log "============================================================"

    rm -rf "${build_dir}" "${install_dir}"

    # Configure
    if cmake -B "${build_dir}" -S "${SRCDIR}" \
        -DCMAKE_INSTALL_PREFIX="${install_dir}" \
        "${cmake_args[@]}" \
        > "${build_dir}_configure.log" 2>&1; then
        log "  configure: OK"
    else
        log "  configure: FAILED (see ${build_dir}_configure.log)"
        return 1
    fi

    # Build
    if cmake --build "${build_dir}" --parallel \
        > "${build_dir}_build.log" 2>&1; then
        log "  build:     OK"
    else
        log "  build:     FAILED (see ${build_dir}_build.log)"
        # Show last 20 lines of error
        tail -20 "${build_dir}_build.log" | sed 's/^/    /' | tee -a "${RESULTS_FILE}"
        return 1
    fi

    # Install
    if cmake --install "${build_dir}" \
        > "${build_dir}_install.log" 2>&1; then
        log "  install:   OK"
    else
        log "  install:   FAILED (see ${build_dir}_install.log)"
        return 1
    fi

    # List installed files
    log "  installed files:"
    find "${install_dir}" -type f | sort | sed "s|${install_dir}/||" | sed 's/^/    /' | tee -a "${RESULTS_FILE}"
}

# -----------------------------------------------------------------------
# Build matrix
# -----------------------------------------------------------------------

COMMON_FLAGS=(
    -DCMAKE_BUILD_TYPE=Debug
    -DAWEMGR_ENABLE_WARNINGS_ARE_ERRORS=ON
)

FAILED=()

run_combo "combo_default" \
    "${COMMON_FLAGS[@]}" \
    -DAWEMGR_BUILD_TESTS=ON \
    -DAWEMGR_BUILD_ADDONS=ON \
    -DAWEMGR_BUILD_SHELL=ON \
    -DAWEMGR_BUILD_TUNING_SERVER=ON \
    -DAWEMGR_BUILD_APPS=ON \
    -DAWEMGR_BUILD_SERVICE=ON \
    -DAWEMGR_AWECORE_CONNECTION=SOCKET \
    -DAWEMGR_LOGGING=STDIO \
    || FAILED+=("combo_default")

run_combo "combo_no_addons" \
    "${COMMON_FLAGS[@]}" \
    -DAWEMGR_BUILD_TESTS=OFF \
    -DAWEMGR_BUILD_ADDONS=OFF \
    -DAWEMGR_BUILD_APPS=OFF \
    -DAWEMGR_AWECORE_CONNECTION=SOCKET \
    -DAWEMGR_LOGGING=STDIO \
    || FAILED+=("combo_no_addons")

run_combo "combo_no_apps" \
    "${COMMON_FLAGS[@]}" \
    -DAWEMGR_BUILD_TESTS=OFF \
    -DAWEMGR_BUILD_ADDONS=ON \
    -DAWEMGR_BUILD_SHELL=ON \
    -DAWEMGR_BUILD_TUNING_SERVER=ON \
    -DAWEMGR_BUILD_APPS=OFF \
    -DAWEMGR_AWECORE_CONNECTION=SOCKET \
    -DAWEMGR_LOGGING=STDIO \
    || FAILED+=("combo_no_apps")

run_combo "combo_no_shell" \
    "${COMMON_FLAGS[@]}" \
    -DAWEMGR_BUILD_TESTS=OFF \
    -DAWEMGR_BUILD_ADDONS=ON \
    -DAWEMGR_BUILD_SHELL=OFF \
    -DAWEMGR_BUILD_TUNING_SERVER=ON \
    -DAWEMGR_BUILD_APPS=ON \
    -DAWEMGR_BUILD_SERVICE=ON \
    -DAWEMGR_AWECORE_CONNECTION=SOCKET \
    -DAWEMGR_LOGGING=STDIO \
    || FAILED+=("combo_no_shell")

run_combo "combo_no_tuning" \
    "${COMMON_FLAGS[@]}" \
    -DAWEMGR_BUILD_TESTS=OFF \
    -DAWEMGR_BUILD_ADDONS=ON \
    -DAWEMGR_BUILD_SHELL=ON \
    -DAWEMGR_BUILD_TUNING_SERVER=OFF \
    -DAWEMGR_BUILD_APPS=ON \
    -DAWEMGR_BUILD_SERVICE=ON \
    -DAWEMGR_AWECORE_CONNECTION=SOCKET \
    -DAWEMGR_LOGGING=STDIO \
    || FAILED+=("combo_no_tuning")

run_combo "combo_single_lib" \
    "${COMMON_FLAGS[@]}" \
    -DAWEMGR_BUILD_TESTS=OFF \
    -DAWEMGR_BUILD_ADDONS=ON \
    -DAWEMGR_BUILD_SHELL=ON \
    -DAWEMGR_BUILD_TUNING_SERVER=ON \
    -DAWEMGR_BUILD_APPS=ON \
    -DAWEMGR_BUILD_SERVICE=ON \
    -DAWEMGR_BUILD_SINGLE_LIB=ON \
    -DAWEMGR_AWECORE_CONNECTION=SOCKET \
    -DAWEMGR_LOGGING=STDIO \
    || FAILED+=("combo_single_lib")

run_combo "combo_shared_lib" \
    "${COMMON_FLAGS[@]}" \
    -DAWEMGR_BUILD_TESTS=OFF \
    -DAWEMGR_BUILD_ADDONS=ON \
    -DAWEMGR_BUILD_SHELL=ON \
    -DAWEMGR_BUILD_TUNING_SERVER=ON \
    -DAWEMGR_BUILD_APPS=ON \
    -DAWEMGR_BUILD_SERVICE=ON \
    -DAWEMGR_BUILD_SHARED_LIB=ON \
    -DAWEMGR_AWECORE_CONNECTION=SOCKET \
    -DAWEMGR_LOGGING=STDIO \
    || FAILED+=("combo_shared_lib")

run_combo "combo_syslog" \
    "${COMMON_FLAGS[@]}" \
    -DAWEMGR_BUILD_TESTS=OFF \
    -DAWEMGR_BUILD_ADDONS=ON \
    -DAWEMGR_BUILD_SHELL=ON \
    -DAWEMGR_BUILD_TUNING_SERVER=ON \
    -DAWEMGR_BUILD_APPS=ON \
    -DAWEMGR_BUILD_SERVICE=ON \
    -DAWEMGR_AWECORE_CONNECTION=SOCKET \
    -DAWEMGR_LOGGING=SYSLOG \
    || FAILED+=("combo_syslog")

run_combo "combo_distribution" \
    -DCMAKE_BUILD_TYPE=Release \
    -DAWEMGR_ENABLE_WARNINGS_ARE_ERRORS=ON \
    -DAWEMGR_BUILD_TESTS=OFF \
    -DAWEMGR_BUILD_ADDONS=ON \
    -DAWEMGR_BUILD_SHELL=ON \
    -DAWEMGR_BUILD_TUNING_SERVER=ON \
    -DAWEMGR_BUILD_APPS=ON \
    -DAWEMGR_BUILD_SERVICE=ON \
    -DAWEMGR_AWECORE_CONNECTION=SOCKET \
    -DAWEMGR_LOGGING=STDIO \
    || FAILED+=("combo_distribution")

# -----------------------------------------------------------------------
# Summary
# -----------------------------------------------------------------------
log ""
log "============================================================"
log "SUMMARY"
log "============================================================"
if [ ${#FAILED[@]} -eq 0 ]; then
    log "All builds PASSED."
else
    log "FAILED builds:"
    for f in "${FAILED[@]}"; do
        log "  - ${f}"
    done
    exit 1
fi
