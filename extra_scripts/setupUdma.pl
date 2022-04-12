#!/usr/bin/perl
# Setup this to run on startup

use warnings;
use strict;

my $BUFSIZE_1=894 * 1024 * 1024;
my $BUFSIZE_2=256 * 1024 * 1024;
my $command="insmod /home/ubuntu/udmabuf/u-dma-buf.ko udmabuf0=$BUFSIZE_2 udmabuf1=$BUFSIZE_1 udmabuf2=$BUFSIZE_2";

exec ($command) or print STDERR "Couldn't exec script: $!";