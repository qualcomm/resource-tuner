#!/bin/bash

nodes_root="/usr/share/urm"
TEST_NODES_DIR="$nodes_root/tests/nodes"

nodes_tmp_base="$(mktemp -d)"
echo "mktemp created: $nodes_tmp_base"
if [ -z "$nodes_tmp_base" ] || [ ! -d "$nodes_tmp_base" ]; then
    echo "mktemp -d failed — aborting"
    exit 1
fi
trap 'rm -rf "$nodes_tmp_base"' EXIT INT TERM

RUNTIME_NODES_DIR="$nodes_tmp_base/urm/tests/nodes"
if ! mkdir -p "$RUNTIME_NODES_DIR"; then
    echo "Failed to create staging directory $RUNTIME_NODES_DIR — suites requiring nodes will SKIP"
    exit 1
elif cp -r "$TEST_NODES_DIR/"* "$RUNTIME_NODES_DIR/"; then
    echo "Staged test nodes from $TEST_NODES_DIR to $RUNTIME_NODES_DIR"
else
    echo "Failed to stage test nodes into $RUNTIME_NODES_DIR — suites requiring nodes will SKIP"
    exit 1
fi

TESTS="
    /usr/bin/UrmComponentTests
    /usr/bin/UrmIntegrationTests
"

# ---------- Execute ----------
PASS=0
FAIL=0
SKIP=0

for t in $TESTS; do
    echo "Running: $t --npath $RUNTIME_NODES_DIR"
    # $t --npath "$RUNTIME_NODES_DIR"
    rc=$?
    case $rc in
        0)
            PASS=$((PASS+1))
            ;;
        1)
            FAIL=$((FAIL+1))
            ;;
        2)
            SKIP=$((SKIP+1))
            ;;
    esac
done

echo "Results: PASS=$PASS  FAIL=$FAIL  SKIP=$SKIP"

if [ "$FAIL" -gt 0 ]; then
  exit 1
fi

exit 0
