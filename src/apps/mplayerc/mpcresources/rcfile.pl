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
# Apply mplayerc.rc file changes to locale rc files.
#
# To use this program, you need a perl with PerlIO package installed.
# For help of usage, type: perl rcfile.pl -h
#

use strict;
use Getopt::Long;
use vars qw(@InTags @TextTags $AdjustedDir);

use File::Basename;
use File::Spec;
use File::Glob qw(:globally :nocase);
require "common.pl";

my $BaseFileName = "../mplayerc.rc";
my $NewFileName = "../mplayerc.rc";
my $help;

my $result = GetOptions("base|b=s" =>\$BaseFileName, "new|n=s" =>\$NewFileName, "help|h"=>\$help);

if($help || !$result) {
	print << 'USAGE';
Usage: perl rcfile.pl [Options] | -h --help
Copy all changes between two version of mplayerc.rc files to all rc files in the current directory,
generate new rc files under the "newrc" subdirectory.

Options:
	--base -b	base file, default "../mplayerc.rc" optional
	--new -n	modified file, default "../mplayerc.rc" optional
	--help -h	show this help

	Base file: the previous revision of mplayerc.rc file
	Modified file: the current mplayerc.rc, all the changes in this file will get copied to the files you chose.

	The examples below assume you are now in the "mpcresources" directory.

	example 1: you want to check if there are differences between mplayerc.rc and mplayerc.language.rc files.
	Go to the mpcresources directory, then 	>perl rcfile.pl

	example 2: you changed some gui of mpc-hc, that means you also modified the mplayerc.rc file.
	First: checkout the head revision of mplayerc.rc using the svn client, give it any other name, for example: 
	>svn cat -r head ../mplayerc.rc > mplayer.rc.old
	Second: >perl rcfile.pl -b mplayerc.rc.old
	Or better yet use provided batch file: >rcfile.bat

	After running this script, you will find all new language rc files under "newrc" subdir. 
	These new rc files have all changes copied from your modified mplayerc.rc file and is ready to compile, 
	except use english version strings. Use rcstrings.pl to extract all strings for translators, 
	After recieved translated text files, use patch.pl script to merge back to rc files.
USAGE
	exit(0);
}

my($BaseDialogs, $BaseMenus, $BaseStrings, @BaseOutline) = ({}, {}, {}, ());
my($NewDialogs, $NewMenus, $NewStrings, @NewOutline) = ({}, {}, {}, ());
my($MenuDiffs, $DialogDiffs) = ({}, {});
my($BaseDesignInfos, $NewDesignInfos) = ({}, {});

my @BaseFile = readFile($BaseFileName, 1);
my @NewFile = readFile($NewFileName, 1);
print "Scanning changes between baseline file and new version...\n\n";
getDifference();

#Trace($BaseDialogs, "Base Dialogs");
#Trace($BaseMenus, "Base Menus");
#Trace($BaseStrings, "Base Strings");
#Trace(\@BaseOutline, "Base Outline");
#Trace($MenuDiffs, "MenuDiffs");
#Trace($DialogDiffs, "DialogDiffs");

my @FileLists = ();

if(@ARGV) {	@FileLists = @ARGV; }
else { @FileLists = <*.rc>; }

#put all generted files under "newrc" sub dir.
if(!-e "newrc"){
	mkdir(File::Spec->catdir(".", "newrc")) || die "Can not create \"newrc\" sub directory.";
}

foreach my $filename(@FileLists) {
	print "Anaylse locale file: $filename...\n";
	my @oldrcfile = readFile($filename, 1);
	my($curDialogs, $curMenus, $curStrings, @curOutline) = ({},{},{}, ());
	my @curVersionInfo = ();
	my $curDesignInfos = {};
	analyseData(\@oldrcfile, \@curOutline, $curDialogs, $curMenus, $curStrings, \@curVersionInfo, $curDesignInfos);
	
	my $newrcfile = File::Spec->catfile(".", "newrc", $filename);

	my @newrc = ();
	my @patches = ();

	writeData(\@newrc, \@patches, \@curOutline, $curDialogs, $curMenus, $curStrings, \@curVersionInfo, $curDesignInfos);

	print "Generate new locale file: $newrcfile...\n\n";
	writeFile($newrcfile, \@newrc, 2);
}

###################################################################################################
sub getDifference {
	my @curVersionInfo = ();	# no use for mplayerc.rc
	analyseData(\@BaseFile, \@BaseOutline,$BaseDialogs, $BaseMenus, $BaseStrings, \@curVersionInfo, $BaseDesignInfos);
	analyseData(\@NewFile,\@NewOutline, $NewDialogs, $NewMenus, $NewStrings, \@curVersionInfo, $NewDesignInfos);

	while (my ($key, $value) = each(%{$BaseMenus})) {
		my $value1 = $NewMenus->{$key};

		if($value1) {
			my @changeset = ();
			lcs($value->{"__TEXT__"}, $value1->{"__TEXT__"}, \@changeset);
			@changeset = @changeset[sort {$changeset[$a][0] <=> $changeset[$b][0];}(0..$#changeset)];
			$MenuDiffs->{$key} = [@changeset];
		}
	}

	while (my ($key, $value) = each(%{$BaseDialogs})) {
		my $value1 = $NewDialogs->{$key};

		if($value1) {
			my @changeset = ();
			my @data0 = map{$_->[1]}@{$value->{"__DATA__"}};
			my @data1 = map{$_->[1]}@{$value1->{"__DATA__"}};
			lcs(\@data0, \@data1, \@changeset);
			@changeset = @changeset[sort {$changeset[$a][0] <=> $changeset[$b][0];}(0..$#changeset)];
			$DialogDiffs->{$key} = [@changeset];
		}
	}

}

#--------------------------------------------------------------------------------------------------
sub writeData {
	my ($newrc, $patches) = (shift, shift);
	my ($curOutline, $curDialogs, $curMenus, $curStrings, $curVersionInfo, $curDesignInfos) = @_;

	my ($curDialogName, $curMenuName, $curDesignName);

	my $headsection = 0;
	my $tailsection = $#NewOutline;
	my $oldtail = @$curOutline - 1;

	my $idx=0;
	foreach (@NewOutline) {
		my $tag = $_->[0];

		if($tag eq "__TEXT__") {
			if($idx==$headsection){
				push(@{$newrc}, @{$curOutline->[0][1]});			# use old language rc file head section
			}
			elsif($idx == $tailsection) {
				writeVersionInfo($newrc, $curVersionInfo);			#TODO: write current version info to it's original place, now just above end section
				push(@{$newrc}, @{$curOutline->[$oldtail][1]});		# use old language rc file head section
			}
			else {
				my @_text = ();
				push(@_text, @{$_->[1]});
				foreach my $line(@_text) {
					foreach my $texttag (@TextTags){
						if($line =~ /\b$texttag\b/) {
							$line =~ s/"res/"${AdjustedDir}res/;	# adjust directory for ICON BITMAP AVI
							last;
						}
					}
				}
				push(@{$newrc}, @_text);	#in general use texts from new rc file
			}
		}
		elsif($tag eq "DESIGNINFO") {
			$curDesignName = $_->[1][0];
			#if($curDesignInfos->{$curDesignName}) {
			#	push(@{$newrc}, @{$curDesignInfos->{$curDesignName}{"__TEXT__"}});	# use locale design info section
			#}
			#else {
				push(@{$newrc}, @{$NewDesignInfos->{$curDesignName}{"__TEXT__"}});	# use new design info section
			#}
		}
		elsif($tag eq "BLOCK") {
			push(@{$newrc}, @{$_->[1]});	# use new file block section
		}
		elsif($tag eq "DIALOG") {
			#use the new rc file dialogs, use locale strings if possible
			$curDialogName = $_->[1][0];
			my @dialogContent = ();
			writeDialogContent(\@dialogContent, $patches, $curDialogs, $curDialogName);
			push(@{$newrc}, @dialogContent);
		}
		elsif($tag eq "MENU") {
			$curMenuName = $_->[1][0];
			my @menuContent = ();
			writeMenuContent(\@menuContent, $patches, $curMenus, $curMenuName);
			push(@{$newrc}, @menuContent);
		}
		elsif($tag eq "STRINGTABLE") {
			#use new rc file stringtables, try to use as many locale strings as possible
			my @newstrings = ();
			push(@newstrings, @{$_->[1]});
			writeStringTable(\@newstrings, $patches, $curStrings);
			@newstrings = grep{$_=~/\S+/;}@newstrings;	#get rid of empty line for patch
			push(@{$newrc}, @newstrings);
		}
		$idx++;
	}
}

#--------------------------------------------------------------------------------------------------
sub writeStringTable {
	my ($output, $patches, $refs) = @_;

	#use new rc file stringtables, try to use as many locale strings as possible
	foreach (@{$output}){
		my ($key, $value);
		
		if (/\b(ID\S+)\b\s*(".+")/){ #distinguish between key value at same line or not for syntax's sake
			($key, $value)= ($1,$2); 
		}
		elsif (/\b(ID\S+)\b\s*$/){ #value too long to fit in one line but we don't care. :)
			$key = $1;
			$value = $NewStrings->{$key};
			$_ = "    $key  $value";
		}
		elsif(/^\s*".+"\s*$/){ #value not same line with key, already dealed with, so just clear it.
			$_ = " ";
			next;
		}
		else {
			next; #other text
		}
			
		my $baseStr = $BaseStrings->{$key};
		my $localeStr = $refs->{$key};
		
		if((!$localeStr) || (!$baseStr) || ($baseStr ne $value)) {
		}
		else {		
			s/\Q$value\E/$localeStr/; #use locale string
		}
	}
}

#--------------------------------------------------------------------------------------------------
sub writeDialogContent {
	my ($output, $patches, $refs, $name) = @_;

	my @contents = ();
	push(@contents, @{$NewDialogs->{$name}{"__TEXT__"}});

	my @newdatas = map{$_->[1]}@{$NewDialogs->{$name}{"__DATA__"}};
	my @newidxs = map{$_->[0]}@{$NewDialogs->{$name}{"__DATA__"}};
	my $newdataLines = @newdatas;

	my @baseidxs = map{$_->[0]}@{$BaseDialogs->{$name}{"__DATA__"}};
	my $basedataLines = @baseidxs;

	my @refdatas = map{$_->[1]}@{$refs->{$name}{"__DATA__"}};
	my $refdataLines = @refdatas;

	if($newdataLines && $basedataLines && $refdataLines) {
		my $diffData = $DialogDiffs->{$name}; 
		
		foreach(@$diffData) {
			my $linenum = $newidxs[$_->[0]];
			
			my $str = $newdatas[$_->[0]];
			my $localstr = $refdatas[$_->[1]];
			
			if($localstr) {
				$contents[$linenum-1] =~ s/\Q$str\E/$localstr/;
			}
		}
	}

	my $localfont = $refs->{$name}{"__FONT__"}[1];
	if($localfont) {
		$contents[$NewDialogs->{$name}{"__FONT__"}[0]-1] = $refs->{$name}{"__TEXT__"}[$refs->{$name}{"__FONT__"}[0]-1];
	}
	
	push(@{$output}, @contents);
}

#--------------------------------------------------------------------------------------------------
sub writeMenuContent {
	my ($output, $patches, $refs, $name) = @_;

	my @contents = ();
	push(@contents, @{$NewMenus->{$name}{"__TEXT__"}});
	my $contentLines = @contents;

	if(my $diffData = $MenuDiffs->{$name}) {
		# this menu exists in old file
		my @changes = grep($_->[0] != $_->[1],@$diffData);	#anything changed for this menu?
		my $samelines = @$diffData; 
		
		if((!@changes) && ($samelines == $contentLines)) { #no change then just use old data
			@contents = ();
			push(@contents, @{$refs->{$name}{"__TEXT__"}}); 
		}
		else {				#change in this menu
			my @checkIdx=(1..$contentLines);

			foreach (@$diffData) {
				$contents[$_->[0]] = $refs->{$name}{"__TEXT__"}[$_->[1]];	#first use those values from locale file
				$checkIdx[$_->[0]] = 0;
			}
		}
	}

	push(@{$output}, @contents);
}

#--------------------------------------------------------------------------------------------------
sub writeVersionInfo{
	my ($input, $versionInfo) = (shift, shift);

	#Trace($versionInfo, "Current VersionInfo");
	push(@{$input}, "");
	push(@{$input}, "/////////////////////////////////////////////////////////////////////////////");
	push(@{$input}, "//");
	push(@{$input}, "// Version");
	push(@{$input}, "//");
	push(@{$input}, @$versionInfo);
}

###################################################################################################
