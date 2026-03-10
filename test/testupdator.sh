#SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
#COLORS_FILE="$SCRIPT_DIR/target-color-config.colors"
#   UPDATE_PROGRAM="$SCRIPT_DIR/color-scheme-udpator"
#   "$UPDATE_PROGRAM" "$SCRIPT_DIR/color-config.txt" "$COLORS_FILE"
A="2"
if [ $A -eq "1" ]; then
    echo "hello"
    A="2"
    echo $A
  else
    echo "bye"
    A="1"
    echo $A
fi