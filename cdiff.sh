#!/bin/bash
# This script performs diff on a csv file, allowing a small tolerance on specified columns

# Colored output
red="", green="", ogColor=""
if tty -s; then
    red=$(tput setaf 1)
    green=$(tput setaf 2)
    ogColor=$(tput sgr0)
fi

echo_red() {
    echo "${red}$*${ogColor}"
}

echo_green() {
    echo "${green}$*${ogColor}"
}

print_help () {
    echo "cdiff - Diff files while allowing a tolerance on specified columns."
    echo ""
    echo "Options:"
    echo "    --column <col3>,<col21>    The column names that can differ"
    echo "    --delimiter <delim>        Default ','"
    echo "    --tolerance <float>        Default 0.1"
    echo "    --verbose                  More output"
    echo "    <file1>                    The base file for comparisons"
    echo "    <file2>                    The file to compare with"
    echo ""
    echo "Usage:"
    echo "    cdiff -c colA,colB -t 0.4 <file1> <file2>"
    echo "    cdiff -c colA -d '|' <file1> <file2>"
}

setup_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            "-c" | "--col" | "--column")
                IFS=',' read -r -a tol_columns <<< "$2"
                shift
                shift
                ;;

            "-d" | "--delim" | "--delimiter")
                delim="$2"
                shift
                shift
                ;;

            "-t" | "--tolerance")
                tolerance=$2
                shift
                shift
                ;;

            "-v" | "--verbose")
                verbose=true
                shift
                ;;

            "-h" | "--help")
                print_help
                exit 0
                ;;

            "-"*)
                echo_red "Unknown argument $1"
                echo ""
                print_help
                exit 1
                ;;
            *)
                file1="$1"
                file2="$2"
                break
                ;;
        esac
    done
}

basic_diff() {
    # If basic diff passes, no need to do column-aware diff
    # When this cmd is within the 'if', it can pass if only one file exists...
    diff -q "$file1" "$file2" > /dev/null 2>&1
    if [[ "$?" == 0 ]]; then
        echo_green "Files are identical."
        exit 0
    fi
}

column_setup() {
    # Get column header string
    header1=$(head -n 1 "$file1")
    header2=$(head -n 1 "$file2")

    # Ensure column headers are equal
    if [[ "$header1" != "$header2" ]]; then
        echo_red "Files have different column headers."
        exit 1
    fi

    # Split header into columns
    IFS="$delim" read -r -a header_columns <<< "$header1"

    # Setup hash table to track error count per column
    for column in "${header_columns[@]}"; do
        column_mismatches[$column]=0
    done

    # Get the data rows
    file1_content=$(cat "$file1")
    file2_content=$(cat "$file2")

    # Split data into an array based on rows
    mapfile -s 1 -t rows_f1 < <(echo "$file1_content")
    mapfile -s 1 -t rows_f2 < <(echo "$file2_content")

    # Ensure files have same number of lines
    if [[ ${#rows_f1[@]} -ne ${#rows_f2[@]} ]]; then
        echo_red "Files have different number of lines!"
        exit 1
    fi
}

column_diff() {
    # Keep track of mismatching rows
    num_rows_invalid=0
    is_row_invalid=false

    # Iterate over data rows, comparing each column
    for (( row = 0; row < num_rows; row++ )); do
        # Turn current row to column array
        IFS="$delim" read -r -a cols_f1 <<< "${rows_f1[row]}"
        IFS="$delim" read -r -a cols_f2 <<< "${rows_f2[row]}"

        # Iterate over each column
        for idx in "${!header_columns[@]}"; do
            # Get values
            column=${header_columns[idx]}
            curr_col1=${cols_f1[idx]}
            curr_col2=${cols_f2[idx]}

            # Check if comparison should be "close enough" or exact
            if [[ " ${tol_columns[*]} " =~ " ${column} " ]]; then
                column_difference=$(awk "BEGIN {print $curr_col1 - $curr_col2}")

                if (( $(echo "${column_difference#-} > $tolerance" | bc -l) )); then
                    is_row_invalid=true
                    (( column_mismatches[$column]++ ))
                fi
            else
                if [[ "$curr_col1" != "$curr_col2" ]]; then
                    is_row_invalid=true
                    (( column_mismatches[$column]++ ))
                fi
            fi
        done

        if [[ "$is_row_invalid" == true ]]; then
            (( num_rows_invalid++ ))

            if $verbose; then
                echo "Line $((row+1)) differs"
            fi
        fi
        is_row_invalid=false

    done
    echo ""
}

print_summary() {
    if [[ $num_rows_invalid -gt 0 ]]; then
        # Construct output for error count per column
        output=""
        for column in "${header_columns[@]}"; do
            if [[ ${column_mismatches[$column]} != 0 ]]; then
                output+="$column=${column_mismatches[$column]}, "
            fi
        done

        echo "Invalid rows    = $num_rows_invalid/$((num_rows-1)) "
        echo "Invalid columns = ${output::-2}"
        echo ""
        echo_red "Files differ more than tolerable."
        exit 1
    else
        echo_green "Files are tolerable."
        exit 0
    fi
}

main() {
    # Vars controlled by script args
    tol_columns=()
    delim=","
    tolerance=0.1
    file1=""
    file2=""
    verbose=false

    # Vars to track stuff between the functions
    declare -a header_columns
    declare -a rows_f1
    declare -a rows_f2
    declare -A column_mismatches

    setup_args "$@"
    basic_diff
    column_setup

    num_rows=${#rows_f1[@]}

    column_diff
    print_summary
}

main "${@:1}" # Pass all args except script name
