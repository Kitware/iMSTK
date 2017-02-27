#!/bin/bash

path=""
path_dest=""

file_path=""
file_path_dest=""

algo_list="sha sha1 sha224 sha256 sha384 sha512 md5"
algo="sha512"

quiet=""

show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-q] -algo -f PATH [-c PATH_DEST]

This script is a convenient script to generate the key file of a file, or files contained in folder sub-folders

- specify the algo preceded by a dash such as -sha512, -md5, etc.
- specify the path to a folder or a file
- /!\ WARNING /!\ : Won't work has expected if a directory contains dot!

Options:

  -h             Display this help and exit.
  -q             Do not output informational messages.
  -algo          Type of algorithm to use to compute the hashsum - by default algo=sha512
  -f             Folder or file target to generate key files
  -c             Destination directory to write the key files - PATH_DEST=PATH by default
EOF
}


while [ $# -gt 0 ]; do
	case "$1" in
		-h)
			show_help
			exit 0
			;;
		-f)
			path=$2
			shift
			;;
		-c)
			path_dest=$2
			shift
			;;
		-q)
			quiet=1 #http://stackoverflow.com/questions/17812340/how-to-put-dev-null-21-into-a-variable
              # Or see https://github.com/thewtex/docker-opengl/blob/master/run.sh#L100
			;;
		*)
      option=`echo "$1" | cut -c 2-` # remove the dash in front of the algo
      #echo $option
      item_found=0
      for item in $algo_list; do
        if [ $item == $option ]; then 
          algo=$option
          item_found=1
        fi
      done

      if ! [ $item_found ]; then
        show_help >&2
			  exit 1
      fi
      ;;
	esac
	shift
done
#echo "COMMAND: ${0##*/} quiet:$quiet algo:$algo PATH:$path PATH_DEST:$path_dest"


# Check path type "directory" or "file"
path_type=""
is_path_dir() {
if [ -d "$1" ]; then
  path_type="directory"
elif [ -f "$1" ]; then
  path_type="file"
else
  path_type=""
fi
}


# create key file
generate_keyFile() {
# compute hash and keep just the hash
content=`$algo"sum" "$file_path" | cut -d ' ' -f 1`
# write the file with the hash
echo -n $content > "$file_path_dest".$algo
}


generate_from_folder() {
list_file="${path%/}/tmp_list_file.txt"
find ${path%/}/* > $list_file
while read -r file_path; do
  file_path_dest="$path_dest/${file_path#$path}"
  if [[ $file_path_dest == *.* ]]; then # if it's a file -> generate the key
    generate_keyFile
  else # if it's a directory -> create it
    mkdir -p "$file_path_dest"
  fi
done < $list_file
rm $list_file
}


# IF the path given is a directory
is_path_dir "$path"
if [ $path_type == "directory" ]; then
  if [ $path_dest == "" ]; then
    path_dest="$path"
  fi
  generate_from_folder
elif [ $path_type == "file" ]; then
  if [ $path_dest == "" ]; then
    path_dest=`dirname "$path"`
  fi
  file_path="$path"
  # create file based on path at path_dest
  mkdir -p "$path_dest"
  file_path_dest="$path_dest/${path##*/}"
  generate_keyFile
fi

