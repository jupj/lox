#!/bin/bash

# expected() returns the expected stdout+stderr for a test file
expected() {
    testfile=$1

    < "$testfile" grep "//" \
        | sed 's/.*\/\/\s*//' \
        | grep -E '^(expect:|expect runtime error:|\[|Error at)' \
        | sed 's/^expect:\s*//' \
        | sed 's/^expect runtime error:\s*//' \
        | sed 's/^\[java /[/'
}

# testloxfile() runs jlox on a file and verifies the output and exit code
testloxfile() {
    testfile=$1

    expectedOut=$(expected "$testfile")
    # remove clox errors:
    expectedOut=$(echo "$expectedOut" | grep -E -v '\[c line')

    expectError=$(grep -E "expect runtime error|\[|Error at" "$testfile") #echo "$expectedOut" | grep -E '^(\[|Error at)')

    echo -n "Testing $testfile ..."

    out=$(java -jar target/jlox-1.0-SNAPSHOT.jar "$testfile" 2>&1)
    code=$?
    out=$(echo "$out" | sed -E 's/\[line [0-9]+\]\s*//')
    expectedOut=$(echo "$expectedOut" | sed -E 's/\[line [0-9]+\]\s*//')

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
        return 0
    else
        echo "FAIL"
        echo "$testerr"
        return 1
    fi
}

testfiles=$(ls craftinginterpreters-test/*.lox; ls craftinginterpreters-test/*/*.lox)
nOK=0
nFAIL=0
nSKIP=0
for testfile in $testfiles; do
    if [[ "$testfile" =~ benchmark|scanning|limit|parse.lox|evaluate.lox ]]; then
        ((nSKIP++))
        echo "Skip $testfile"
        continue
    fi

    if testloxfile "$testfile"; then
        ((nOK++))
    else
        ((nFAIL++))
    fi
done
echo "Total: $nOK OK, $nFAIL failed, $nSKIP skipped"
