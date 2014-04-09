PSPPATH="/cygdrive/$1/PSP/GAME/"
mkdir .psp 
cd .psp 
../unpack-pbp ../EBOOT.PBP 
mkdir $PSPPATH"$3" 
mv UNKNOWN.PSP $PSPPATH"$3"/EBOOT.PBP 
mkdir $PSPPATH"$2%" 
touch UNKNOWN.PSP 
#cp ../data/pic1.png .
#cp ../data/icon0.png .
../_pack-pbp $PSPPATH"$2%/EBOOT.PBP" PARAM.SFO ICON0.PNG ICON1.PMF UKNOWN.PNG PIC1.PNG SND0.AT3 UNKNOWN.PSP UNKNOWN.PSAR 
#touch -t 200101010000 $PSPPATH"$2"
touch  $PSPPATH"$3"
touch  $PSPPATH"$2%"
#mv $2* .. 
cd .. 
rm -rf .psp 
