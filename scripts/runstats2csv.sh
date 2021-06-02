#! /bin/bash

# reads a file in which each line contains a tag and a value and converts it
# to a csv file where
#   - there is a column for each distinct tag, with that tag as header
#   - all values having that tag are in the column, in order of appearance
# for example, if the file has a collection of lines of the form
#      runtime VALUE
# the output will have a column with header runtime and all instances of VALUE below it.
# [from mfms/Utilities repository]

function usage {
    echo "Usage: $1 INPUT_FILE TAGS_FILE [DELIMITER]"
    echo " each line of INPUT_FILE is assumed to contain a tag and a value"
    echo " the TAGS_FILE contains a list of tags whose values are to be harvested (max 99)"
    echo " output (stdout) is a csv file with a column for each tag, with values below it"
    echo " DELIMITER is the separator between tag and value in the input, white space if not specified"
}

if [ $# -lt 2 ] || [ $# -gt 3 ]; then
    usage
    exit
fi

input_file=$1
shift
tags_file=$1
shift
delimiter=$1

if [ -n "$delimiter" ]; then
    delimiter_flag=-F$delimiter
fi
    
seq_no=0
for tag in `cat $tags_file`; do
    seq_string=`echo $seq_no | awk '{printf "%02d", $1}'`
    column_file=/tmp/$$-$seq_string-$tag
    echo $tag > $column_file
    grep "^$tag" $input_file | awk $delimiter_flag '{print $2}' >> $column_file
    seq_no=$((seq_no + 1))
done

paste -d, /tmp/$$-[0-9][0-9]-*

#  [Last modified: 2021 05 31 at 20:29:42 GMT]
