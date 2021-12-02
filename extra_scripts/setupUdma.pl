#!/usr/bin/perl
# Setup this to run on startup

use warnings;
use strict;

my $BUFSIZE=128 * 1024 * 1024;
my $command="insmod /home/ubuntu/udmabuf/u-dma-buf.ko udmabuf0=$BUFSIZE udmabuf1=$BUFSIZE udmabuf2=$BUFSIZE udmabuf3=$BUFSIZE udmabuf4=$BUFSIZE udmabuf5=$BUFSIZE";

exec ($command) or print STDERR "Couldn't exec script: $!";