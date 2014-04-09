#!/usr/bin/sh
DIR1="s9xTYL~1"
DIR2="s9xTYL                         1"
#DIR1=s9xTYL
#DIR2=s9xTYL
./deploy.sh $1 "$DIR1" "$DIR2"
if [ "$2" = "reset" ]
then
	rm /cygdrive/$1/psp/game/"$DIR2"/s9xTYL.ini
fi
