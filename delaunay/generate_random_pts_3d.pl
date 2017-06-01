#!/usr/bin/perl


use warnings;
use strict;

my $numpts = shift(@ARGV) || 1000;

print(<<EOF);
# vtk DataFile Version 1.0
3D triangulation data
ASCII

DATASET POLYDATA
EOF

printf("POINTS %d float\n", $numpts);
    

foreach my $i (1..$numpts) {
    printf("%f %f %f\n", rand(1000.0), rand(1000.0), rand(1000.0));
}

