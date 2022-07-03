#!/bin/bash
rm -f test_S.txt test_U.txt
for (( i=3; i <= 80; i++ ))
do
    echo -en "\n $i: " >> test_S.txt
    echo -en "\n $i: " >> test_U.txt
    for (( c=0; c<1;  c++ ))
    do
        python map_gen.py $i -u -i $(( $i * ($RANDOM % 400) )) > test
        clang++ -Wall -Werror -Wextra main.cpp && ./a.out -nh test >> test_U.txt
        python map_gen.py $i -s -i $(( $i * ($RANDOM % 400) )) > test
        clang++ -Wall -Werror -Wextra main.cpp && ./a.out -nh test >> test_S.txt        
    done
done
