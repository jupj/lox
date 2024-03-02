#!/bin/bash

# expected() returns the expected stdout+stderr for a test file
expected() {
    testfile=$1

    < "$testfile" grep "//" \
        | sed 's/.*\/\/\s*//' \
        | grep -E '^(expect:|\[)' \
        | sed 's/^expect:\s*//' \
        | sed 's/^\[java /[/'
}

# testloxfile() runs jlox on a file and verifies the output and exit code
testloxfile() {
    testfile=$1

    expectedOut=$(expected "$testfile")
    expectError=$(echo "$expectedOut" | grep -E '^\[')

    echo -n "Testing $testfile ..."

    out=$(java -jar target/jlox-1.0-SNAPSHOT.jar "$testfile" 2>&1)
    code=$?

    testerr=$(
        if [ "$out" != "$expectedOut" ]; then
            echo "*** Got unexpected output:"
            echo "$out"
            echo "*** Expected:"
            echo "$expectedOut"
        fi

        if [ "$code" -eq 0 ]; then
            if [ "$expectError" != "" ]; then
                echo "*** Expected error, got no error"
            fi
        else
            if [ "$expectError" = "" ]; then
                echo "*** Got error $code, expected no error"
            fi
        fi
    )

    if [ "$testerr" = "" ]; then
        echo "OK"
    else
        echo "" # Add end of line
        echo "$testerr"
    fi
}

testloxfile "../craftinginterpreters/test/empty_file.lox"
testloxfile "../craftinginterpreters/test/precedence.lox"
testloxfile "../craftinginterpreters/test/unexpected_character.lox"
testloxfile "../craftinginterpreters/test/operator/not.lox"
