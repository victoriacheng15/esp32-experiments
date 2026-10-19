#!/usr/bin/env bash
set -euo pipefail

# ==============================================================================
# Script: edit-video.sh
# Purpose: Fast, deterministic video trimming and web-optimization for physical
#          hardware and breadboard demonstration recordings.
#
# Concept (Start, End, and Duration):
#   - <start_sec>:   Lead-in cut. Discards initial setup dead air (phone mounting,
#                    pressing record, hands moving into frame).
#   - <end_sec>:     Trailing cut by timestamp. Discards trailing dead air
#                    (reaching to stop recording, bench cleanup).
#   - -d <duration>: Length cut by seconds. Captures a fixed number of seconds
#                    starting from <start_sec> without calculating the end time.
#   - Audio is stripped (-an) by default to eliminate background room noise.
#
# Example:
#   You record a 35s raw clip (raw_bench.mp4).
#   - 0s to 5s: Hands entering frame, adjusting power.
#   - 5s to 20s: Turning potentiometer and observing LED patterns (THE DEMO).
#   - 20s to 35s: Reaching to phone screen to tap stop recording.
#
#   Method A (By End Timestamp): "Start at 5s, stop at 20s"
#     ./scripts/edit-video.sh raw_bench.mp4 5 20
#
#   Method B (By Duration): "Start at 5s, capture for 15s"
#     ./scripts/edit-video.sh raw_bench.mp4 -s 5 -d 15
#
#   Both methods output the identical 15-second demo clip.
#
#   To preview commands without executing:
#     ./scripts/edit-video.sh raw_bench.mp4 5 20 --dry-run
# ==============================================================================

usage() {
  cat << 'EOF'
Usage:
  # Fast positional syntax:
  edit-video.sh <input.mp4> <start_sec> <end_sec> [flags]

  # Flag-based syntax:
  edit-video.sh <input.mp4> -s <start_sec> -d <duration_sec> [flags]

Flags:
  -o, --output <path>    Output destination (default: <base>_edited.mp4)
  -d, --duration <sec>   Duration in seconds instead of end timestamp
  --keep-audio           Preserve original audio (default: audio stripped)
  --dry-run              Print planned ffmpeg command without executing
  -h, --help             Display this usage summary
EOF
  exit 1
}

INPUT=""
START_SEC=""
END_SEC=""
DURATION_SEC=""
OUTPUT=""
STRIP_AUDIO=true
DRY_RUN=false
POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    -i|--input) INPUT="$2"; shift 2 ;;
    -s|--start) START_SEC="$2"; shift 2 ;;
    -e|--end) END_SEC="$2"; shift 2 ;;
    -d|--duration) DURATION_SEC="$2"; shift 2 ;;
    -o|--output) OUTPUT="$2"; shift 2 ;;
    --keep-audio) STRIP_AUDIO=false; shift ;;
    --dry-run) DRY_RUN=true; shift ;;
    -h|--help) usage ;;
    -*) echo "[ERROR] Unknown option: $1" >&2; usage ;;
    *) POSITIONAL+=("$1"); shift ;;
  esac
done

# Resolve positional parameters
if [[ -z "${INPUT}" && ${#POSITIONAL[@]} -ge 1 ]]; then
  INPUT="${POSITIONAL[0]}"
fi
if [[ -z "${START_SEC}" && ${#POSITIONAL[@]} -ge 2 ]]; then
  START_SEC="${POSITIONAL[1]}"
fi
if [[ -z "${END_SEC}" && -z "${DURATION_SEC}" && ${#POSITIONAL[@]} -ge 3 ]]; then
  END_SEC="${POSITIONAL[2]}"
fi

[[ -z "${INPUT}" || -z "${START_SEC}" ]] && usage
[[ -z "${END_SEC}" && -z "${DURATION_SEC}" ]] && usage

if [[ "${DRY_RUN}" == false && ! -f "${INPUT}" ]]; then
  echo "[ERROR] Input file not found: ${INPUT}" >&2
  exit 1
fi

if [[ -z "${OUTPUT}" ]]; then
  EXT="${INPUT##*.}"
  BASE="${INPUT%.*}"
  OUTPUT="${BASE}_edited.${EXT}"
fi

# Build ffmpeg arguments
FFMPEG_ARGS=(-ss "${START_SEC}" -i "${INPUT}")

if [[ -n "${END_SEC}" ]]; then
  DURATION=$(echo "${END_SEC} - ${START_SEC}" | bc -l)
  FFMPEG_ARGS+=(-t "${DURATION}")
else
  FFMPEG_ARGS+=(-t "${DURATION_SEC}")
fi

if [[ "${STRIP_AUDIO}" == true ]]; then
  FFMPEG_ARGS+=(-an)
else
  FFMPEG_ARGS+=(-c:a aac -b:a 128k)
fi

FFMPEG_ARGS+=(
  -c:v libx264
  -preset fast
  -crf 18
  -pix_fmt yuv420p
  -movflags +faststart
  "${OUTPUT}"
)

if [[ "${DRY_RUN}" == true ]]; then
  echo "[DRY-RUN] Planned execution:"
  echo "  Input:     ${INPUT}"
  echo "  Start:     ${START_SEC}s"
  if [[ -n "${END_SEC}" ]]; then
    echo "  End:       ${END_SEC}s (Duration: ${DURATION}s)"
  else
    echo "  Duration:  ${DURATION_SEC}s"
  fi
  echo "  Output:    ${OUTPUT}"
  echo "  Command:   ffmpeg -y ${FFMPEG_ARGS[*]}"
  exit 0
fi

# Execute transcode
ffmpeg -y "${FFMPEG_ARGS[@]}"

echo "[SUCCESS] Saved trimmed clip to: ${OUTPUT}"
