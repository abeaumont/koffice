#!/usr/bin/perl
# a script for use by autoconf to make the Makefiles
# from the Makefile.in's
#
# the original autoconf mechanism first splits all substitutions into groups
# of ca. 90, and than invokes sed for _every_ Makefile.in and every group
# (so around 2-3 times per Makefile.in). So this takes forever, as sed
# has to recompile the regexps every time.
#
# this script does better. It changes all Makefile.ins in one process.
# in kdelibs the time for building Makefile went down from 2:59 min to 13 sec!
#
# written by Michael Matz <matz@ifh.de>
#
# the first part was done by looking at the config.status files generated
# by configure.
# 
my $ac_cs_root=$ARGV[0];
my $ac_given_srcdir=$ARGV[1];
my $ac_given_INSTALL=$ARGV[2];

# print "ac_cs_root=$ac_cs_root\n";
# print "ac_given_srcdir=$ac_given_srcdir\n";
# print "ac_given_INSTALL=$ac_given_INSTALL\n";

my ($srcdir, $top_srcdir);
my $INSTALL;
my $bad_perl = ($] < 5.005);

open(CF, "< $ac_cs_root.subs") || die "can't open $ac_cs_root.subs: $!";
my @subs = <CF>;
close(CF);
chomp @subs;
@comp_match=();
@comp_subs=();

if ($bad_perl) {
    print "Using perl older than version 5.005\n";
    foreach my $pat (@subs) {
	if (  ($pat =~ /s%([^%]*)%([^%]*)%g/ )
	   || ($pat =~ m%/([^/]*)/([^/]*)/g% )
	   || ($pat =~ /s%([^%]*)%([^%]*)%;t/ )
	   || ($pat =~ m%/([^/]*)/([^/]*)/;t% )
	   ) {
            # form : s%bla%blubb%g
	    # or     s%bla%blubb%;t t   (newer autoconf)
	    push @comp_subs, make_closure($1, $2);
	} elsif ( ($pat =~ /%([^%]*)%d/ )
	   || ($pat =~ m%/([^/]*)/d% )
	   ) {
	    push @comp_subs, make_closure($1, "");
	} else {
	    die "Uhh. Malformed pattern in $ac_cs_root.subs ($pat)"
		unless ( $pat =~ /^\s*$/ );   # ignore white lines
	}
    }
} else {
    foreach my $pat (@subs) {
	if (  ($pat =~ /s%([^%]*)%([^%]*)%g/ )
	   || ($pat =~ m%/([^/]*)/([^/]*)/g% )
	   || ($pat =~ /s%([^%]*)%([^%]*)%;t/ )
	   || ($pat =~ m%/([^/]*)/([^/]*)/;t% )
	   ) {
            # form : s%bla%blubb%g
	    # or     s%bla%blubb%;t t   (newer autoconf)
	    push @comp_match, eval "qr/\Q$1\E/";  # compile match pattern
	    push @comp_subs, $2;
	} elsif ( ($pat =~ /%([^%]*)%d/ )
	   || ($pat =~ m%/([^/]*)/d% )
	   ) {
	    push @comp_match, eval "qr/\Q$1\E/";
	    push @comp_subs, "";
	} else {
	    die "Uhh. Malformed pattern in $ac_cs_root.subs ($pat)"
		unless ( $pat =~ /^\s*$/ );   # ignore white lines
	}
    }
}
undef @subs;

# read the list of files to be patched, form:
# ./Makefile arts/Makefile arts/examples/Makefile arts/flow/Makefile

open(CF, "< $ac_cs_root.sacfiles") || die "can't open $ac_cs_root.sacfiles: $!";
my @ac_files = <CF>;
close(CF);
chomp @ac_files;

my $ac_file;
foreach $ac_file (@ac_files) {
    next if $ac_file =~ /\.\./;
    next if $ac_file =~ /^\s*$/;
    my $ac_file_in;
    my ($ac_dir, $ac_dots, $ac_dir_suffix);

    if ($ac_file =~ /.*:.*/ ) {
	($ac_file_in = $ac_file) =~ s%[^:]*:%%;
	$ac_file =~ s%:.*%%;
    } else {
	$ac_file_in = $ac_file.".in";
    }

# Adjust a relative srcdir, top_srcdir, and INSTALL for subdirectories.

# Remove last slash and all that follows it.  Not all systems have dirname.
    ($ac_dir = $ac_file) =~ s%/[^/][^/]*$%%;
    if ( ($ac_dir ne $ac_file) && ($ac_dir ne ".")) {
# The file is in a subdirectory.
	if (! -d "$ac_dir") { mkdir "$ac_dir", 0777; }
	($ac_dir_suffix = $ac_dir) =~ s%^./%%;
	$ac_dir_suffix="/".$ac_dir_suffix;
# A "../" for each directory in $ac_dir_suffix.
	($ac_dots = $ac_dir_suffix) =~ s%/[^/]*%../%g;
    } else {
	$ac_dir_suffix="";
	$ac_dots="";
    }

    if ($ac_given_srcdir eq ".") {
	$srcdir=".";
	if ($ac_dots) {
	    ( $top_srcdir = $ac_dots) =~ s%/$%%;
	} else { $top_srcdir="."; }
    } elsif ($ac_given_srcdir =~ m%^/%) {
	$srcdir=$ac_given_srcdir.$ac_dir_suffix;
	$top_srcdir = $ac_given_srcdir;
    } else {
	$srcdir = $ac_dots.$ac_given_srcdir.$ac_dir_suffix;
	$top_srcdir = $ac_dots.$ac_given_srcdir;
    }

    if ($ac_given_INSTALL) {
	if ($ac_given_INSTALL =~ m%^/% ) {
	    $INSTALL = $ac_given_INSTALL;
	} else {
	    $INSTALL = $ac_dots.$ac_given_INSTALL;
	}
    }

    print "fast creating $ac_file\n";
    unlink $ac_file;
    my $ac_comsub="";
    my $fname=$ac_file_in;
    $fname =~ s%.*/%%;
    my $configure_input="Generated automatically from $fname by config.pl.";
    if ($ac_file =~ /.*[Mm]akefile.*/) {
	$ac_comsub="# ".$configure_input."\n";  # for the first line in $ac_file
    }

    my $ac_file_inputs;
    ($ac_file_inputs = $ac_file_in) =~ s%^%$ac_given_srcdir/%;
    $ac_file_inputs =~ s%:% $ac_given_srcdir/%g;

    patch_file($ac_file, $ac_file_inputs, $ac_comsub);
}

sub patch_file {
    my ($outf, $infiles, $firstline) = @_;
    my $filedata;
    my @infiles=split(' ', $infiles);
    my $i=0;

    if ($firstline) {
	$filedata = $firstline;
    }
    foreach my $name (@infiles) {
	if (open(CF, "< $name")) {
	    while (<CF>) {
		$filedata .= $_;
	    }
	    close(CF);
	} else {
	    print STDERR "can't open $name: $!"."\n";
	}
    }

    $filedata =~ s%\@configure_input\@%$configure_input%g;
    $filedata =~ s%\@srcdir\@%$srcdir%g;
    $filedata =~ s%\@top_srcdir\@%$top_srcdir%g;
    $filedata =~ s%\@INSTALL\@%$INSTALL%g;

    if ($bad_perl) {
	while ($i <= $#comp_subs) {
	    my $ref = $comp_subs[$i];
	    &$ref(\$filedata);
	    $i++;
	}
    } else {
	while ($i <= $#comp_match) {
	    $filedata =~ s/$comp_match[$i]/$comp_subs[$i]/g;
	    $i++;
	}
    }
    open(CF, "> $outf") || die "can't create $outf: $!";
    print CF $filedata;
    close(CF);
}

sub make_closure {
    my ($pat, $sub) = @_;
    $pat =~ s/\@/\\@/g;   # @bla@ -> \@bla\@
    $pat =~ s/\$/\\\$/g;  # $bla -> \$bla
    $sub =~ s/\@/\\@/g;
    $sub =~ s/\$/\\\$/g;
    my $ret = eval "return sub { my \$ref=shift; \$\$ref =~ s%$pat%$sub%g; }";
    if ($@) {
        print "can't create CODE: $@\n";
    }
    return $ret;
}
