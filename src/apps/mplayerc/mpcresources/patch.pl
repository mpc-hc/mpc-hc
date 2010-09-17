#/bin/perl
#
# $Id$
#
# (C) 2006-2010 see AUTHORS
#
# This file is part of mplayerc.
#
# Mplayerc is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Mplayerc is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Apply translated text strings to mplayerc.language.rc file
#
# To use this program, you need a perl with PerlIO package installed.
# For help of usage, type: perl patch.pl -h

use strict;
use PerlIO::encoding;
use Data::Dumper;
use Getopt::Long;
use vars qw(@InTags);

require "common.pl";

my $TxtFileName = "mplayerc.language.rc.txt";
my $help;

my $result = GetOptions("input|i=s" =>\$TxtFileName, "help|h"=>\$help);

if($help || !$result) {
	print << 'USAGE';
Usage: perl patch.pl -i translate.txt rcfile | -h --help
Read translated strings file, apply translation to rc file.

Options:
	--input -i	translated strings file
	--help -h	show this help
	rcfile		rc file to apply the translation to

USAGE
	exit(0);
}

my($Dialogs, $Menus, $Strings) = ({}, {}, {}, ());
my($NewDialogs, $NewMenus, $NewStrings) = ({},{},{});
my @Outline = ();
my @VersionInfo = ();

print "Reading rc file...\n";
my $rcfile = shift(@ARGV);
my @RcFile = readFile($rcfile, 1);
analyseData(\@RcFile, \@Outline, $Dialogs, $Menus, $Strings, \@VersionInfo);

print "\nReading string texts file...\n";
my @TxtFile = readFile($TxtFileName, 1);
analyseTxt(\@TxtFile, $NewDialogs, $NewMenus, $NewStrings);

print "\nWriteing new rc file...\n";
my @newrc = ();
mergeData(\@newrc, \@RcFile);
writeFile($rcfile, \@newrc, 2); # overwrite rcfile

###################################################################################################
sub mergeData {
	my ($newrc, $rcfile) = @_;

	my ($curDialogName, $curMenuName);

	foreach (@Outline) {
		my $tag = $_->[0];

		if($tag eq "__TEXT__") {
				push(@{$newrc}, @{$_->[1]});	# write text section
		}
		elsif($tag eq "BLOCK") {
			push(@{$newrc}, @{$_->[1]});		# write block section
		}
		elsif($tag eq "VERSIONINFO") {
			push(@{$newrc}, @{$_->[1]});		# write block section
		}
		elsif($tag eq "DIALOG") {
			$curDialogName = $_->[1][0];
			my @dialogContent = ();
			mergeDialog(\@dialogContent, $NewDialogs, $Dialogs, $curDialogName);
			push(@{$newrc}, @dialogContent);
		}
		elsif($tag eq "MENU") {
			$curMenuName = $_->[1][0];
			my @menuContent = ();
			mergeMenu(\@menuContent, $NewMenus, $Menus, $curMenuName);
			push(@{$newrc}, @menuContent);
		}
		elsif($tag eq "STRINGTABLE") {
			my @strings = ();
			push(@strings, @{$_->[1]});
			mergeStringTable(\@strings, $NewStrings);
			push(@{$newrc}, @strings);
		}
	}
}
#--------------------------------------------------------------------------------------------------
sub analyseTxt {
	my ($input, $dialogs, $menus, $strings) = @_;

	my @inputs=();
	push(@inputs, @{$input});
	@inputs = grep{$_=~/\S+/;}@inputs;		#get rid of empty line
	@inputs = map{$_=~s/\s*$//;$_}@inputs;	#remove newline

	my $bInBlock=0;
	my @data = ();
	my ($tag, $name, $linenum) = ("", "", 0);

	foreach (@inputs) {
		if(/\bBEGIN\b/) {
			$bInBlock = 1;
			($tag, $name, $linenum) = ($_=~/(DIALOGEX|MENU)\s+(\S+)\s+LINES\s+(\d+)/);
		}
		elsif(/\bEND\b/) {
			$bInBlock = 0;
			if($tag eq "DIALOGEX") {
				$dialogs->{$name}->{"__DATA__"}=[@data];
				$dialogs->{$name}->{"__LINES__"} = $linenum;
			}
			else {
				$menus->{$name}->{"__DATA__"}=[@data];
				$menus->{$name}->{"__LINES__"} = $linenum;
			}
			@data=();
		}
		else {
			if($bInBlock) {
				my ($l, $v) = ($_=~/^\s*(\d+)\s*(".*")\s*$/);
				push(@data, [$l, $v]);
			}
			else {
				if(/^\s*STRING\s*(\S+)\s*(".*")\s*$/) {
					$strings->{$1} = $2;
				}
			}
		}
	}
}

#--------------------------------------------------------------------------------------------------
sub mergeStringTable {
	my ($output, $patches) = @_;

	#if string id found in patches, use translated strings
	foreach (@{$output}){
		my($key, $value) = ($_=~/\b(ID\S+)\b\s*(".+")/);

		my $localeStr = $patches->{$key};

		if($localeStr) {
			s/\Q$value\E/$localeStr/;
		}
	}
}

#--------------------------------------------------------------------------------------------------
sub mergeMenu {
	my ($output, $patches, $refs, $name) = @_;

	my @contents = ();
	push(@contents, @{$refs->{$name}{"__TEXT__"}});
	my $contentLines = @contents;

	my $curlines = $refs->{$name}{"__LINES__"};
	my $newlines = $patches->{$name}{"__LINES__"};

	if($newlines == $curlines) {
		my @data = @{$patches->{$name}{"__DATA__"}};

		foreach(@data) {
			my $line = $_->[0];
			my $value = $_->[1];
			$contents[-- $line] =~ s/".*"/$value/;
		}
	}

	push(@{$output}, @contents);
}

#--------------------------------------------------------------------------------------------------
sub mergeDialog {
	my ($output, $patches, $refs, $name) = @_;

	my @contents = ();
	push(@contents, @{$refs->{$name}{"__TEXT__"}});
	my $contentLines = @contents;

	my $curlines = $refs->{$name}{"__LINES__"};
	my $newlines = $patches->{$name}{"__LINES__"};

	if($newlines == $curlines) {
		my @data = @{$patches->{$name}{"__DATA__"}};

		foreach(@data) {
			my $line = $_->[0];
			my $value = $_->[1];
			my $curline = $contents[--$line];
			$curline = skipNonTranslatedStr($curline);
			$curline=~/("[^"](?:[^"]|"")*")/;
			$contents[$line] =~ s/\Q$1\E/$value/;
		}
	}

	push(@{$output}, @contents);
}

###################################################################################################
