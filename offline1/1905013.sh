#!/bin/bash
submission=$1
target=$2
tests=$3
answers=$4

mkdir "$target/C"
mkdir "$target/Python"
mkdir "$target/Java"
csv_file="$target/result.csv"
echo "${student_id},${type},${matched},${not_matched}" > "$csv_file"

if [[ "$5" == *"-v"* ]]; then
  echo "Verbose mode enabled"
  set -v  
fi
for zipfile in "$submission"/*.zip; do
   if [ -f "$zipfile" ]; then
   delimeter='_'
   parsed_string=$( echo "$zipfile" | cut -d "$delimeter" -f 5)
    delimeter='.'
   parsed_string=$( echo "$parsed_string" | cut -d "$delimeter" -f 1)
   echo "$parsed_string"
   unzip "$zipfile" -d "$submission"
   
   #copies the c file into target directory 
  if find "$submission" -type f -name "*.c" | grep -q "."; then
  echo "C file found"
  mkdir "$target/C/$parsed_string"
  #searching for the file and if found moving to the target folder
   find "$submission" -type f -name "*.c" -exec mv {} "$target/C/$parsed_string" \;
   #renaming the file
   find "$target/C/$parsed_string" -type f -name "*.c" -exec sh -c 'mv "$0" "${0%/*}/'"main.c"'"' {} \;

   if [[ "$6" == "-noexecute" ]]; then
  echo "Skipping execution"
  continue  
fi 

   #now compile and run the code
   gcc "$target/C/$parsed_string/main.c" -o "$target/C/$parsed_string/main"
   if [ $? -eq 0 ]; then
i=1
   for testcase in "$tests"/*.txt; do
   ./$target/C/$parsed_string/main < "$testcase" > "$target/C/$parsed_string/out$i.txt"
   i=$(expr $i + 1)
   done
   fi
   #compile and run done
   #
   #
   #csv works........
   m=0
u=0

   for ansfile in "$answers"/*.txt; do
   position=11
   delimeter='.'
   parsed_string2=$( echo "$ansfile" | cut -d "$delimeter" -f 1)
   echo $parsed_string2
letter="${parsed_string2:$position:1}"
fileout="$target/C/$parsed_string/out$letter.txt"
output=$(diff "$ansfile" "$fileout")

 if [ -z "$output" ]; then
m=$(expr $m + 1)
 fi
 u=$(expr $u + 1)
   done
 u=$(expr $u - $m)
  echo $m
 echo $u
   data="${parsed_string},C,${m},${u}"
   echo $data
   echo "done------------------------"
   echo "$data" >> "$csv_file"
   ############csv_ends###############
   fi


   #copies the python file into target directory
    if find "$submission" -type f -name "*.py" | grep -q "."; then
  echo "python file found"
  mkdir "$target/Python/$parsed_string"
   find "$submission" -type f -name "*.py" -exec mv {} "$target/Python/$parsed_string" \;
      find "$target/Python/$parsed_string" -type f -name "*.py" -exec sh -c 'mv "$0" "${0%/*}/'"main.py"'"' {} \;

 #now compile and run the code
   
i=1
   for testcase in "$tests"/*.txt; do
   python3 "$target/Python/$parsed_string/main.py" < "$testcase" > "$target/Python/$parsed_string/out$i.txt"
   i=$(expr $i + 1)
   done
m1=0
u1=0
   #compile and run done
for ansfile in "$answers"/*.txt; do
   position=11
   delimeter='.'
   parsed_string2=$( echo "$ansfile" | cut -d "$delimeter" -f 1)
   echo $parsed_string2
letter="${parsed_string2:$position:1}"
echo "$letter"
fileout="$target/Python/$parsed_string/out$letter.txt"
output=$(diff "$ansfile" "$fileout")

 if [ -z "$output" ]; then
m1=$(expr $m1 + 1)
 fi
 u1=$(expr $u1 + 1)
 echo $m1
 echo $u1
   done
  echo $m1
 u1=$(expr $u1 - $m1)
 echo $u1
   data="${parsed_string},Python,${m1},${u1}"
   echo $data
   echo "$data" >> "$csv_file"
   ############csv_ends###############



   fi
    #copies the java file into target directory
    if find "$submission" -type f -name "*.java" | grep -q "."; then
  echo "java file found"
  mkdir "$target/Java/$parsed_string"
   find "$submission" -type f -name "*.java" -exec mv {} "$target/Java/$parsed_string" \;
      find "$target/Java/$parsed_string" -type f -name "*.java" -exec sh -c 'mv "$0" "${0%/*}/'"main.java"'"' {} \;

     #compiling 
javac "$target/Java/$parsed_string/main.java" 
   if [ $? -eq 0 ]; then
   echo "java file compiled nicely"
   i=1
   for testcase in "$tests"/*.txt; do
java -cp "$target/Java/$parsed_string" "Main" < "$testcase"> "$target/Java/$parsed_string/out$i.txt"
i=$(expr $i + 1)
   done
   fi

   #compiling done
m2=0
u2=0
   for ansfile in "$answers"/*.txt; do
   position=11
   delimeter='.'
   parsed_string2=$( echo "$ansfile" | cut -d "$delimeter" -f 1)
   echo $parsed_string2
letter="${parsed_string2:$position:1}"
echo "$letter"
fileout="$target/Java/$parsed_string/out$letter.txt"
output=$(diff "$ansfile" "$fileout")

 if [ -z "$output" ]; then
m2=$(expr $m2 + 1)
 fi
 u2=$(expr $u2 + 1)
 echo $m2
 echo $u2
   done
  echo $m2
 u2=$(expr $u2 - $m2)
 echo $u2
   data="${parsed_string},Java,${m2},${u2}"
   echo $data
   echo "$data" >> "$csv_file"
   ############csv_ends###############

   fi
#---done-------
   fi
   done
set +v