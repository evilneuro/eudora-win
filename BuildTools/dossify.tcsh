cat $1 | sed 's/'`echo "\x0A"`'/'`echo "\x0D\x0A"`'/' >$1.tmp
mv $1.tmp $1

