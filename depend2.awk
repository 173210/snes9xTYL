BEGIN {
	cpt=0;
}
/\:/ {source_file=$1;}
/.\// {if (cpt) {depends_from[cpt++]=$1;} else {cpt=1;}}
END {
	new_source = substr(source_file,3)
	gsub ( "\\.cpp",".o",new_source);	
	gsub ( "\\.c",".o",new_source);	
	printf "%s ", new_source >> "depends.txt" ;
	for (i in depends_from) printf "%s ",substr(depends_from[i],3) >> "depends.txt" ;
	printf "\n" >> "depends.txt" ;
}