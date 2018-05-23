rem @echo off
p4 submit -c %1
if not errorlevel 1 perl -x -S %0 %1 %2 | p4 change -f -i
@exit/b

#!perl -w

$new_changelist = shift;
$release_name = shift;

$new_desc = `p4 describe -s $new_changelist`;
$new_desc =~ /\n\tIntegrated from Change (\d+)[^\n]+\n\n/;
$orig_changelist = $1;

$desc = `p4 change -o $orig_changelist`;
$desc =~ s/(\nDescription:\n\t)\[.*\] */$1/;
$desc =~ s/(\nDescription:\n\t)/$1\[$release_name:$new_changelist\] /;

#print STDERR "$desc\n";
print $desc;
