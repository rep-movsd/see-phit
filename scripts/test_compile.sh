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
  g++ -I.. -S --std=c++14 $i.cpp ../seephit.cpp 2>&1 >/dev/null | grep ParseError\(\" > $i.err
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
    if ! (diff $i.err $i.expected) > /dev/null ; then
      printf "${NC}$i - ${RED}Failed\n"
    else
      printf "${NC}$i - ${BLUE}Passed\n"
      rm $i.err
    fi
  fi
done
  
printf "\n${BLUE}Done${NC}\n\n"


