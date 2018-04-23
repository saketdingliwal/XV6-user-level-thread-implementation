if [[ $# -ne 2 ]]
then
echo "Usage: ./submit.sh <entry number> <path to report in pdf>"
exit 1
fi

mkdir $1
cp syscall.h $1
cp syscall.c $1
cp sysproc.c $1
cp proc.c $1
cp usys.S $1
cp user.h $1
cp $2 $1
tar -czvf $1.tar.gz $1
echo "Submit $1.tar.gz on Moodle."

rm -rf $1

