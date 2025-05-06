# shellcheck shell=ash
# shellcheck disable=SC2009 # Do not consider pgrep
# shellcheck disable=SC1091 # Do not follow sourced files
[ -f /etc/environment ] && . /etc/environment

# Set core dump size according to environment
ulimit -c "${ULIMIT_CONFIGURATION:-0}"

LOG_TAG="amx_init"

# Wrapper function to add log tags
_log(){
    logger -t "$LOG_TAG" "$*"
}

# Get uptime in seconds.
_uptime_s(){
    cut -d " " -f 1 < /proc/uptime
}

# Remove the decimal separator.
_uptime_wo_decimal_seperator(){
    _uptime_s | tr -d "."
}

# /proc/uptime units are seconds, but they are values with fractions and proc(5) manual does not specify the
#   fractional part. Because we don't have math functions with fractions, we count the digits after
#   decimal separator and bring all time values to same scale.
_seconds_to_uptime_scale(){
    local time_s="$1"
    local exponent

    # Find out how many digits are after decimal separator
    exponent="$(_uptime_s | cut -d "." -f 2 | tr -d "\n" | wc -c)"

    # echo "$((time_s*(10**exponent)))"
    # Calculate time_s*10^n in a portable way
    for _ in $(seq "$exponent"); do
        time_s=$((time_s*10))
    done
    echo "$time_s"
}

# Wait for a specific process to stop.
_process_shutdown_wait(){
    local name="$1"
    local timeout_s="${2:-5}"

    if [ -z "$name" ]; then
        return
    fi

    start_uptime="$(_uptime_wo_decimal_seperator)"
    current_uptime="$start_uptime"

    # Increase the timeout incase process_shutdown times out and SIGKILL needs some time to cleanup
    timeout_s="$((timeout_s*2))"
    timeout_scaled="$(_seconds_to_uptime_scale "$timeout_s")"

    while [ $((current_uptime-start_uptime)) -lt "$timeout_scaled" ] \
        && ls "/var/run/${name}.pid.kill" >/dev/null 2>&1; do

        sleep 0.1 2>/dev/null || sleep 1
        current_uptime="$(_uptime_wo_decimal_seperator)"
    done
}

# Start an amxrt process
#
# usage: process_boot <name> [opts ...]
#
#   name    - name of the component
#   opts    - further options will be passed to component
process_boot(){
    process_start "$@"
}

# Start an amxrt process
#
# usage: process_start <name> [opts ...]
#
#   name    - name of the component
#   opts    - further options will be passed to component
process_start(){
    local name="$1"
    shift

    ${name} "$@"
}

# Stop an amxrt process
#   process will be stopped by a SIGTERM signal.
#   if process does not stop in [timeout_sec] seconds
#   it will be killed with a SIGKILL
#
# usage: process_stop <name> [timeout_sec]
#
#   name        - name of the component
#   timeout_sec - gracefully stop timeout. Default: 5
process_stop(){
    local name="$1"
    local timeout_s="${2:-5}"

    process_shutdown "$name" "$timeout_s"
    _process_shutdown_wait "$name" "$timeout_s"
}

# Stop an amxrt process in background
#   process will be stopped by a SIGTERM signal.
#   if process does not stop in [timeout_s] seconds
#   it will be killed with a SIGKILL
#   
# usage: process_shutdown <name> [timeout_s]
#
#   name        - name of the component
#   timeout_s   - gracefully stop timeout. Default: 15
process_shutdown(){
    local name="$1"
    local timeout_s="${2:-15}"
    local timeout_scaled
    local pid
    local current_uptime
    local start_uptime
    local time_stamp

    time_stamp="$(_uptime_s)"

    if [ -f "/var/run/${name}.pid" ]; then
        pid=$(cat "/var/run/${name}.pid")
    elif [ -f "/var/run/${name}/${name}.pid" ]; then
        pid=$(cat "/var/run/${name}/${name}.pid")
    fi

    if [ -z "$pid" ]; then
        _log "A PID file for process $name not found. Nothing to stop..."
        return
     elif [ ! -d "/proc/$pid" ]; then
        _log "Process $name - PID $pid is not running. Nothing to stop..."
        return
    elif ! grep -q "$name" "/proc/$pid/status"; then
        _log "Process name of PID $pid does not match $name. Nothing to stop..."
        return
    fi

    _log "Sending SIGTERM to process $name - $pid..."
    kill "$pid"

    # Create a file that marks process shutdown is in progress
    touch "/var/run/${name}.pid.kill"

    # Wait for process to stop in background
    (
        start_uptime="$(_uptime_wo_decimal_seperator)"
        current_uptime="$start_uptime"
        timeout_scaled="$(_seconds_to_uptime_scale "$timeout_s")"

        # Wait for the process to terminate (with timeout)
        while [ $((current_uptime-start_uptime)) -lt "$timeout_scaled" ] && [ -d "/proc/$pid" ]; do
            sleep 0.1 2>/dev/null || sleep 1
            current_uptime="$(_uptime_wo_decimal_seperator)"
        done

        # Check if the process is still running
        if [ -d "/proc/$pid" ]; then
            # Send SIGKILL
            kill -9 "$pid"
            _log "Process $name - $pid did not terminate within $timeout_s seconds. Sending SIGKILL..."

            # Wait until process is removed (with timeout)
            start_uptime="$(_uptime_wo_decimal_seperator)"
            current_uptime="$start_uptime"

            # Give SIGKILL some time to cleanup
            timeout_s="$((timeout_s/2))"
            [ "$timeout_s" -lt 1 ] && timeout_s=1
            timeout_scaled="$(_seconds_to_uptime_scale "$timeout_s")"

            while [ $((current_uptime-start_uptime)) -lt "$timeout_scaled" ] && [ -d "/proc/$pid" ]; do
                sleep 0.1 2>/dev/null || sleep 1
                current_uptime="$(_uptime_wo_decimal_seperator)"
            done
        else
            _log "Process $name - $pid stopped."
        fi
        _log "Process stop for $name - $pid started at ${time_stamp} and finished at $(_uptime_s)"

        # Process shutdown finished. Remove the mark file
        rm "/var/run/${name}.pid.kill"
    )&
}

# Wait for all background shutdown processes
#
# usage: process_shutdown_wait_all <name> [timeout_s]
#
#   name        - name of the component
#   timeout_s   - gracefully stop timeout. Default: 15
process_shutdown_wait_all(){
    local timeout_s="${1:-15}"
    local timeout_scaled
    local current_uptime
    local start_uptime

    start_uptime="$(_uptime_wo_decimal_seperator)"
    current_uptime="$start_uptime"
    timeout_scaled="$(_seconds_to_uptime_scale "$timeout_s")"

    while ls /var/run/*.pid.kill >/dev/null 2>&1; do
        if [ $((current_uptime-start_uptime)) -gt "$timeout_scaled" ]; then
            echo "Timeout occoured while waiting for following services to shutdown:" | tee /dev/console | logger -t "$LOG_TAG"
            find /var/run/ -name "*.pid.kill" | tee /dev/console | logger -t "$LOG_TAG"
            return
        fi
        sleep 0.1 2>/dev/null || sleep 1
        current_uptime="$(_uptime_wo_decimal_seperator)"
    done
}

# Show DM entries
#
# usage: process_debug_info <datamodel_root> [protected_datamodel] [depth]
#
#   datamodel_root      - name of the process
#   protected_datamodel - show protected DM. Default: true
#   depth               - depth of the DM, default is deepest
process_debug_info(){
    datamodel_root="$1"
    protected_datamodel="${2:-true}"
    depth="${3:-}"

    if $protected_datamodel ; then
        ba-cli "ubus-protected; pcb-protected; ${datamodel_root}.?${depth}"
    else
        ba-cli "${datamodel_root}.?${depth}"
    fi
}

# DEPRECATION NOTICE
# This function is deprecated. Instead, one of process_stop or process_shutdown functions should be used.
#
# Stop an amxrt process
#   process will be stopped by a SIGTERM signal.
#   if process does not stop in [timeout_sec] seconds
#   it will be killed with a SIGKILL
#
# usage: process_stop_shutdown <name> [timeout_sec]
#
#   name        - name of the component
#   timeout_sec - gracefully stop timeout. Default: 5
process_stop_shutdown(){
    local name="$1"
    local timeout_s="${2:-5}"
    echo "Warning! Deprecated function process_stop_shutdown called by ${name}!" | tee /dev/console | logger -t "$LOG_TAG"
    process_stop "$name" "$timeout_s"
}
