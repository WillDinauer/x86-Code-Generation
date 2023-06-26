make && ./codegen $1 | tee out.s | pygmentize -f terminal 2>/dev/null && ./assemble.sh out.s && ./a.out; echo "return value from main: "$?
