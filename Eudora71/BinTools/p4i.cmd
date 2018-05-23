@echo off
p4 integ "%2" -s //...@%1,@%1 //...
p4 describe -s %1 > origdesc
p4 change -o | perl -x -S %0 origdesc | p4 change -i
del origdesc
p4 resolve -am //...
exit/b
#!perl -w

while (<STDIN>)
{
	if (/^\t<enter description here>/)
	{
		while (<>)
		{
			if (/^\t/)
			{
				s/^\t\[.*\] */\t/;
				print;
			}
			elsif (/^Change /)
			{
				$origInfo = $_;
			}
		}
		print "\t\n\tIntegrated from $origInfo";
	}
	else
	{
		print;
	}
}

