#!/usr/bin/perl
use strict;

# Expect histogram file as first argument, and objdump file as second.

# Read whole dump file into $dump.
#print "About to read dump file.\n";
open my $fh, '<', $ARGV[1] or die "Can't open file $!";
chomp(my @dump = <$fh>);
close $fh;
my $di = 0;
my $instruction;
my $benchname = $ARGV[0];
$benchname =~ s/.*(4\d\d\.[^.-]+).*/$1/;
my @lines;

open(HISTO, '<', $ARGV[0]) or die $!;
chomp(@lines = <HISTO>);
close HISTO;
#print "Finished sort.\n";
foreach my $line ( @lines ) {
        if ($line =~ /^(\d+)\,(\d+)/) {
                my $addr = $1;
                my $hex = lc (sprintf("%X", $addr));
                #print "$hex = $count\n";
                #if ($dump =~ /^\w+$hex\:(\w[[:xdigit:]]{2})+\w+(\W+)/) {
                $instruction = "";
                my $imm = 1;
                while ($instruction eq "" && $di != @dump) {
                        if ($dump[$di] =~ /\s+$hex\:(\s[[:xdigit:]]{2})+\s+(\S+)/) {
                                $instruction = $2;
                                if ($dump[$di] =~ /\s\-?(\d+)$/) {
                                        $imm = $1 + 1;
                                        $instruction .= "i";
                                }
                                
                        }
                        $di++;
                }
                if ($di == @dump) { $di = 0; }
                while ($instruction eq "" && $di != @dump) {
                        if ($dump[$di] =~ /\s+$hex\:(\s[[:xdigit:]]{2})+\s+(\S+)/) {
                                $instruction = $2;
                                if ($dump[$di] =~ /\s\-?(\d+)$/) {
                                        $imm = $1 + 1;
                                        $instruction .= "i";
                                }
                                
                        }
                        $di++;
                }
                print $line, ",$instruction\n";
                
        }
}