#!/bin/sh

# Ensure projects directory exists
if [ ! -d "projects" ]; then
    echo "Error: 'projects/' directory not found."
    exit 1
fi

# Collect subdirectories in projects/
PROJECTS=""
for dir in projects/*/; do
    if [ -d "$dir" ]; then
        folder=$(basename "$dir")
        PROJECTS="${PROJECTS}${folder} "
    fi
done

if [ -z "$PROJECTS" ]; then
    echo "No projects found inside projects/"
    exit 1
fi

# Convert list to positional parameters
set -- $PROJECTS
NUM_PROJECTS=$#
SELECTED=1

# ANSI Formatting Codes
HIDE_CURSOR="\033[?25l"
SHOW_CURSOR="\033[?25h"
CLEAR_LINE="\033[2K\r"
MOVE_UP="\033[1A"
BOLD_BLUE="\033[1;34m"
RESET="\033[0m"

# Restore cursor and terminal settings on exit/interrupt
cleanup() {
    stty "$OLD_TTY_SETTINGS" 2>/dev/null
    printf "$SHOW_CURSOR\r\n"
}
trap 'cleanup; exit 0' INT TERM EXIT

# Function to render menu (using \r\n for raw mode alignment)
render_menu() {
    printf "${BOLD_BLUE}? Select active ESP-IDF project:${RESET} (Use Arrow keys or j/k, Enter to select)\r\n"
    i=1
    for item in "$@"; do
        if [ "$i" -eq "$SELECTED" ]; then
            printf "  ${BOLD_BLUE}❯ %s${RESET}\r\n" "$item"
        else
            printf "    %s\r\n" "$item"
        fi
        i=$((i + 1))
    done
}

# Function to clear previous menu rendering cleanly
clear_menu() {
    lines=$((NUM_PROJECTS + 1))
    while [ "$lines" -gt 0 ]; do
        printf "$MOVE_UP$CLEAR_LINE"
        lines=$((lines - 1))
    done
}

# Save original TTY settings and configure raw mode
OLD_TTY_SETTINGS=$(stty -g)
stty raw -echo

# Initial Render
printf "$HIDE_CURSOR"
render_menu "$@"

# Main Key Loop
while true; do
    # Read 1 byte at a time
    char=$(dd bs=1 count=1 2>/dev/null)

    # Check for Escape sequence byte (\033)
    if [ "$char" = "$(printf '\033')" ]; then
        char2=$(dd bs=1 count=1 2>/dev/null)
        char3=$(dd bs=1 count=1 2>/dev/null)
        
        case "$char3" in
            A) action="UP" ;;
            B) action="DOWN" ;;
        esac
    else
        case "$char" in
            k|K) action="UP" ;;
            j|J) action="DOWN" ;;
            "") action="ENTER" ;;
            $(printf '\r')|$(printf '\n')) action="ENTER" ;;
            $(printf '\003')) cleanup; exit 0 ;; # Ctrl+C
        esac
    fi

    # Handle Navigation Actions
    case "$action" in
        UP)
            if [ "$SELECTED" -gt 1 ]; then
                SELECTED=$((SELECTED - 1))
            else
                SELECTED=$NUM_PROJECTS
            fi
            ;;
        DOWN)
            if [ "$SELECTED" -lt "$NUM_PROJECTS" ]; then
                SELECTED=$((SELECTED + 1))
            else
                SELECTED=1
            fi
            ;;
        ENTER)
            break
            ;;
    esac

    action=""
    clear_menu
    render_menu "$@"
done

cleanup

# Get selected project name
eval "CHOICE=\${$SELECTED}"
PROJECT_PATH="projects/$CHOICE"

# Recreate relative symlinks at root
rm -f main CMakeLists.txt
ln -s "$PROJECT_PATH/main" main
ln -s "$PROJECT_PATH/CMakeLists.txt" CMakeLists.txt

printf "${BOLD_BLUE}✔ Active project switched to: %s${RESET}\n" "$CHOICE"
