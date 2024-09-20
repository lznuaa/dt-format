# declare a function named '_'
_(){

    # create two temp files

    A=$(mktemp)
    B=$(mktemp)

    # process files with sed
    # if you want, you can do more processing here

    [ -f $1 ] && dt-format $1 > $A
    [ -f $2 ] && dt-format $2 > $B

    # echo the unprocessed file names

    echo $2;

    diff -u  -B $A $B | sed '1,2d' | expand -i -t 4

    # remove tempfiles

    rm $A

    rm $B
}

# execute the function '_' with provided args. $1 = old_file, $2 = curr_file
_ $@

