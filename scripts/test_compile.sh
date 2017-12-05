#!/bin/bash


RED='\033[1;31m'
BLUE='\033[1;34m' 
NC='\033[0m' # No Color

printf "${BLUE}Compiling\n"
printf "${BLUE}----------------------------------------------------------------\n"

cd ../test

for i in *.spt; 
do 
  node ../scripts/make_test.js $i > $i.cpp;
done

for i in *.spt; 
do
  printf "${NC}$i\n"
  time g++ -I.. -S --std=c++17 -Wall $i.cpp &> /tmp/$i.err
  printf "\n"
  
  if (egrep -o "((Warning\(\) \[.*)|(Error\(\) \[.*))" /tmp/$i.err > /tmp/$i.perr); then
    mv /tmp/$i.perr ./$i.err
  else
    mv /tmp/$i.err ./$i.err
  fi
done

rm *.cpp
rm *.s

echo
printf "${BLUE}Checking expected compiler output\n"
printf "${BLUE}----------------------------------------------------------------\n"

for i in *.spt; 
do
  if [ ! -f $i.expected ]; then
    printf "${NC}$i - ${RED}Test file $i.expected not found\n"
  else
    if ! (diff -b $i.err $i.expected) > /dev/null ; then
      ERR=$(cat $i.err)
      printf "${NC}$i - ${RED}Failed: $ERR\n"
    else
      ERR=$(node ../scripts/filter_err.js "$(cat $i.err)")
      printf "${NC}$i - ${BLUE}Passed: $ERR\n"
      rm $i.err
    fi
  fi
done
  
printf "\n${BLUE}Done${NC}\n\n"


