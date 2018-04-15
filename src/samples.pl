#!/usr/bin/perl

use strict;
use warnings;

use List::Util;

use constant PI    => 4 * atan2(1, 1);

my $cluster_count = $ARGV[0] || 2;
my $samples_count = $ARGV[1] || 200;
my $s_per_c = ($samples_count - $cluster_count) / $cluster_count;
$s_per_c = 1 if $s_per_c < 1;

print STDERR "generate $cluster_count clusters in $samples_count samples\n";

my @clusters = generate_cluster_centers($cluster_count);
my @samples;
while(my $cluster = shift @clusters) {
    my $s_count = @clusters ? int($s_per_c * (0.6 + 0.8 * rand())) : ($samples_count - scalar(@samples));
    print STDERR "Cluser # $cluster->[3]: $cluster->[0], $cluster->[1] ($cluster->[2]) $s_count samples\n";
    push @samples, generate_samples($s_count, $cluster);
}
@samples = List::Util::shuffle @samples;

foreach my $s (@samples) {
    print "$s->[0];$s->[1]\n";
}

sub generate_cluster_centers {
    my $cluster_count = shift;
    my @clusters;
    for my $n (1..$cluster_count) {
        push @clusters, [rand(150) - 75, rand(150) - 75, rand(), $n];
    }

    return @clusters;
}

sub generate_samples {
    my $samples_count = shift;
    my $cluster = shift;

    my @samples;
    while(scalar(@samples) < $samples_count) {
        my $d = rand(10) * $cluster->[3];
        my $a = rand(2 * PI);
        my $s = [$d * sin($a), $d * cos($a)];

        my $k = $d;
        $s->[0] = $s->[0] * $k + $cluster->[0];
        $s->[1] = $s->[1] * $k + $cluster->[1];

        if($s->[0] < 100 && $s->[0] > -100 && $s->[1] < 100 && $s->[1] > -100) {
            push @samples, [@$s, $cluster->[3]];
        }
    }
    return @samples;
}

sub dist { sqrt(sqr($_[0][0] - $_[1][0]) + sqr($_[0][1] - $_[1][1])) }

sub sqr { $_[0] * $_[0] }