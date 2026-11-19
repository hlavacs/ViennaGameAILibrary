#!/usr/bin/env bash
set -euo pipefail

# Starts the demos of the Vienna Game AI Library on Linux and macOS.
#
# Usage: ./run_demo.sh [name]
#   name   one of the demos listed below, with or without the prefix "demo_",
#          upper or lower case, for example: ./run_demo.sh mcts
#   Without a name the script shows a menu.
#
# Build the demos first with build_linux.sh or build_macos.sh. The demos are
# started from the repository root, because they load their images and fonts
# as Demo/res/... relative to that folder.

cd "$(dirname "$0")"

DEMOS="Pathfinding DecisionTree StateMachine SteeringBehaviors Flocking MCTS SeekAndFlee PursueAndEvade Arrive Wander"

# --- Find the folder with the demo programs ---
# The build scripts set VGAI_DEMO_DIR when they start a demo themselves.
# Otherwise the first build directory that contains the demos is used.
DEMO_DIR="${VGAI_DEMO_DIR:-}"
if [ -z "$DEMO_DIR" ]; then
  for candidate in build/release-linux/Demo build/debug-linux/Demo build/release-macos/Demo build/debug-macos/Demo build/Demo; do
    if [ -x "$candidate/demo_Flocking" ]; then
      DEMO_DIR="$candidate"
      break
    fi
  done
fi
if [ -z "$DEMO_DIR" ] || [ ! -x "$DEMO_DIR/demo_Flocking" ]; then
  printf 'The demos were not found. Build them first with ./build_linux.sh or ./build_macos.sh\n' >&2
  exit 1
fi

# Turns a menu number or a name in any spelling into the name of a demo.
# Prints nothing if there is no such demo.
resolve_demo() {
  local wanted number=0 demo
  wanted="$(printf '%s' "$1" | tr '[:upper:]' '[:lower:]')"
  wanted="${wanted#demo_}"
  for demo in $DEMOS; do
    number=$((number + 1))
    if [ "$wanted" = "$number" ] || [ "$wanted" = "$(printf '%s' "$demo" | tr '[:upper:]' '[:lower:]')" ]; then
      printf '%s' "$demo"
      return 0
    fi
  done
}

start_demo() {
  local demo
  demo="$(resolve_demo "$1")"
  if [ -z "$demo" ] || [ ! -x "$DEMO_DIR/demo_$demo" ]; then
    printf 'There is no demo "%s" in "%s".\n' "$1" "$DEMO_DIR" >&2
    return 1
  fi
  printf 'Starting demo_%s ...\n' "$demo"
  "$DEMO_DIR/demo_$demo"
}

if [ "$#" -gt 0 ]; then
  start_demo "$1"
  exit $?
fi

while true; do
  printf '\nDemos of the Vienna Game AI Library  [%s]\n\n' "$DEMO_DIR"
  printf '  Main themes\n'
  printf '   1  Pathfinding         left click: start, right click: target\n'
  printf '   2  DecisionTree        watch the guard and the burglar decide\n'
  printf '   3  StateMachine        watch the worker change its state\n'
  printf '   4  SteeringBehaviors   watch chickens, dog and snake; day and night change\n'
  printf '   5  Flocking            click a box, type a number, press Enter\n'
  printf '   6  MCTS                Connect Four against the search; keys are shown in the window\n\n'
  printf '  Single steering behaviours\n'
  printf '   7  SeekAndFlee         arrow keys move the target\n'
  printf '   8  PursueAndEvade      arrow keys move the target\n'
  printf '   9  Arrive              left click sets the target\n'
  printf '  10  Wander              watch\n\n'
  printf 'Number or name, Enter to quit: '
  choice=""
  read -r choice || true
  if [ -z "$choice" ] || [ "$choice" = "q" ]; then
    exit 0
  fi
  start_demo "$choice" || true
done
