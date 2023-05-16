#!/bin/bash

counter=0
limit=10

while [ $counter -ne $limit ]
do
    ./a.out > output.txt

    if [ $? -ne 0 ]; then
        echo "Fail"
        exit 1
    #else
        #echo "Success"
    fi

    counter=$((counter+1))
done

echo "Success"
exit 0
