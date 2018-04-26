#!/usr/bin/perl

use strict;
use warnings;

use feature qw/state/;

use List::Util;

use constant PI => 4 * atan2(1, 1);

my $cluster_count = $ARGV[0] || 2;
my $samples_count = $ARGV[1] || 200;

print STDERR "generate $cluster_count clusters in $samples_count samples\n";

my @clusters = generate_cluster_centers($cluster_count);
my @samples;
while(my $cluster = shift @clusters)
{
    my $s_count = $samples_count - scalar(@samples);
    if(@clusters) {
        $s_count = int($s_count * (0.5 + rand()) / scalar(@clusters + 1));
    }

    print STDERR "Cluser # $cluster->[3]: $cluster->[0], $cluster->[1] ($cluster->[2]) $s_count samples\n";
    push @samples, generate_samples($s_count, $cluster);
}
@samples = List::Util::shuffle @samples;

foreach my $s (@samples)
{
    print "$s->[0];$s->[1]\n";
}

sub generate_cluster_centers {
    my $cluster_count = shift;
    my @clusters;
    for my $n (1..$cluster_count)
    {
        push @clusters, [150*rand() - 75, 150*rand() - 75, gauss(1.0, 0.8), $n];
    }

    return @clusters;
}

sub generate_samples {
    my $samples_count = shift;
    my $cluster = shift;

    my @samples;
    while(scalar(@samples) < $samples_count)
    {
        my $x = $cluster->[0] + 10 * $cluster->[2] * gauss(0, 1);
        my $y = $cluster->[1] + 10 * $cluster->[2] * gauss(0, 1);

        if($x < 100 && $x > -100 && $y < 100 && $y > -100) {
            push @samples, [$x, $y, $cluster->[3]];
        }
    }
    # while(scalar(@samples) < $samples_count)
    # {
    #     my $d = 10 * $cluster->[2] + gauss(1, 0.5);
    #     my $a = 2 * PI * rand();
    #     my $x = $cluster->[0] + $d * sin($a);
    #     my $y = $cluster->[1] + $d * cos($a);

    #     if($x < 100 && $x > -100 && $y < 100 && $y > -100) {
    #         push @samples, [$x, $y, $cluster->[3]];
    #     }
    # }
    return @samples;
}

sub gauss {
    my ($mean, $dev) = @_;
    $mean //= 0.0;
    $dev //= 1.0;

    state $next;

    my $result;
    if(defined($next)) {
        $result = $next;
        $next = undef;
    } else {
        my $a = 2 * PI * rand();
        my $d = sqrt(-2 * log(rand()));
        $result = cos($a) * $d;
        $next = sin($a) * $d;
    }
    return $result * $dev + $mean;
}