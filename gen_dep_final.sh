rm depends.txt
find . -name "*.dep" -exec awk -f depend2.awk -F \" {} \;