/\#include / {depends_from[cpt++]=$2;}
END {
	str_depfile=sprintf("%s.dep",ARGV[1]);
	printf "%s:\n",ARGV[1] > str_depfile ;
	for (i in depends_from) {
		/*printf(" %s",depends_from[i]);*/
		str_cmd=sprintf("find . -name %s >> %s",depends_from[i],str_depfile);
		out=system(str_cmd);
	}
}