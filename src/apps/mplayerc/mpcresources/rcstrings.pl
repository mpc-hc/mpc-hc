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
# Extract all translatable strings from rc files.
#
# To use this program, you need a perl with PerlIO package installed.
# For help of usage, type: perl rcstrings.pl -h
#

use strict;
use Getopt::Long;
use vars qw(@InTags @TextTags);

use File::Basename;
use File::Spec;
use File::Glob qw(:globally :nocase);
require "common.pl";

my $Extension=".txt";
my $help;
my ($OutputAll, $OutputDialogs, $OutputMenus, $OutputStringtables);

my $result = GetOptions("suffix|x=s"=>\$Extension, "help|h"=>\$help, "all|a"=>\$OutputAll,
												"dialog|d"=>\$OutputDialogs, "menu|m"=>\$OutputMenus, "stringtable|s"=>\$OutputStringtables);

if($OutputAll) {
	($OutputDialogs, $OutputMenus, $OutputStringtables) = (1,1,1);
}

if($help || !$result) {
	print << 'USAGE';
Usage: perl rcstrings.pl [Options] file1 file2 | -h --help
Extract all translatable strings from file1 file2 or all rc files.

Options:
	--suffix -x	output file suffix, default ".txt" optional
	--help -h	show this help

	-all -a	output all strings, including dialogs, menus, stringtables
	-dialog -d	output dialogs
	-menu -m	output menus
	-stringtable -s	output stringtables

	After running this script, you will find all the string text files under "text" sub directory.
USAGE
	exit(0);
}

my @FileLists = ();

if(@ARGV) {	@FileLists = @ARGV; }
else { @FileLists = <*.rc>; }

#put all generated files under text sub dir.
if(!-e "text"){
	mkdir(File::Spec->catdir(".", "text")) || die "Cannot create \"text\" sub directory.";
}

foreach my $filename(@FileLists) {
	print "Analyzing locale file: $filename...\n";
	my @rcfile = readFile($filename, 1);
	my($curDialogs, $curMenus, $curStrings, @curOutline) = ({},{},{}, ());
	my @curVersionInfo = ();
	my $curDesignInfos = {};
	analyseData(\@rcfile, \@curOutline, $curDialogs, $curMenus, $curStrings, \@curVersionInfo, $curDesignInfos);

	my $txtfile = File::Spec->catfile(".", "text", $filename.$Extension);

	writeFileStrings($txtfile, $curDialogs, $curMenus, $curStrings);
}

###################################################################################################
sub writeFileStrings {
	my ($filename, $dialogs, $menus, $strings) = @_;
	my @contents = ();

	if($OutputDialogs) {
		foreach (sort(keys(%{$dialogs}))) {
			my @data = ();
			push(@data, @{$dialogs->{$_}{"__DATA__"}});
			if(defined($data[0])) {
				push(@contents, ["DIALOG", {$_ =>[@data], "__LINES__" => $dialogs->{$_}{"__LINES__"}}]);
			}
		}
	}
	if($OutputMenus) {
		foreach (sort(keys(%{$menus}))) {
			my @data = ();
			push(@data, @{$menus->{$_}{"__DATA__"}});
			@data = grep(skipNonTranslatedStr($_->[1]), @data);
			@data = grep(($_->[1] !~/""/),@data);
			push(@contents, ["MENU",{$_ =>[@data], "__LINES__" => $menus->{$_}{"__LINES__"}}]);
		}
	}
	if($OutputStringtables) {
		foreach (sort(keys(%{$strings}))) {
			my $line = $strings->{$_};
			$line = skipNonTranslatedStr($line);
			if($line) {
				push(@contents,["STRINGTABLE",{$_=>$strings->{$_}}]);
			}
		}
	}

	if(@contents) {
		print "Generating string files $filename...\n";
		writePatchFile($filename, \@contents, 1);
	}
}

###################################################################################################
