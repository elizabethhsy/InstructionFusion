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
#print "Read dump file.\n";

my %instructions = ();
my @cincoffsetimms;
my @csetboundsimms;

open(HISTO, '<', $ARGV[0]) or die $!;
my %histo;
while (my $line = <HISTO>) {
        if ($line =~ /^(\d+)\,(\d+)/) {$histo{$1}=$2;}
}
close HISTO;
#%histo = sort { $a <=> $b } keys %histo;
#print "Finished sort.\n";
my $logscale = 5;
my $scale = 2**$logscale;
print "scale: $scale , logscale: $logscale \n";
foreach my $addr (sort { $a <=> $b } keys %histo) {
        my $hex = lc (sprintf("%X", $addr));
        my $count = $histo{$addr};
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
                        #if ($dump[$di] =~ /csp.*csp/) {
                        #        $instruction .= "ss";
                        #} elsif ($dump[$di] =~ /csp/) {
                        #        $instruction .= "s";
                        #}
                        
                }
                $di++;
        }
        #print "$dump[$di] ______ $instruction \n";
        if ($di == @dump) { $di = 0; }
        else {
                $instructions{$instruction} += $count;
                if ($instruction eq "cincoffseti") {
                        my $idx = int(log($imm)/log(2) +  0.9999);
                        if ((($imm-1) % $scale) == 0 && $idx >= $logscale) { $idx = $idx - $logscale; }
                        $cincoffsetimms[$idx] += $count;
                }
                if ($instruction eq "csetboundsi" && (($imm-1) % 2 == 0)) {
                        $imm = int(log($imm)/log(2) +  0.9999);
                        #print $dump[$di-1]." $imm\n";
                        $csetboundsimms[$imm] += $count;
                }
        }
}
#foreach my $instruction (sort keys %instructions) {
#        print "$benchname\,$instruction\,$instructions{$instruction}\n";
#}
print "incoffseti,", join(',', @cincoffsetimms), "\n";
print "setboundsi,", join(',', @csetboundsimms), "\n";
