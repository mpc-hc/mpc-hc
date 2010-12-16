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
# Common function for rcfile.pl & patch.pl
#
# To use this program, you need a perl with PerlIO package installed.
#

use strict;
use PerlIO::encoding;
use Data::Dumper;

use vars qw(@InTags @TextTags $AdjustedDir);

@InTags = qw(DIALOGEX MENU STRINGTABLE VERSIONINFO DIALOG DLGINIT TOOLBAR);
@TextTags = qw(BITMAP AVI ICON);
$AdjustedDir = q(..\\\\);

1;

###################################################################################################
sub analyseData {
	my ($input, $outline) = (shift, shift);
	my ($dialogs, $menus, $strings, $versionInfo, $designInfos) = @_;

	my @inputs=();
	push(@inputs, @{$input});

	my $bInBlock=0;

	my ($stack, $tagidx, $curline) = (0,0, "");

	my @blocks=();
	my @text=();

	foreach (@inputs) {
		chop;chop;

		$curline=$_;
		if(!$bInBlock) {
			foreach my $tag(@InTags) {
				if($curline=~/\b$tag\b/) {
					$bInBlock=1;
					last;
				}
				$tagidx++;
			}
		}

		if($bInBlock) {
			push(@blocks, $_);

			if(@text) {
				push(@{$outline}, ["__TEXT__",[@text]]);
				@text=();
			}

			if(/\bBEGIN\b/) {	$stack++;	}
			elsif(/\bEND\b/) {
				$stack--;

				if($stack==0) {
					if($tagidx == 0) {
						my $dlgname = readDialog($dialogs, \@blocks);
						push(@{$outline}, ["DIALOG", [$dlgname, ""]]);
					}
					elsif($tagidx == 1) {
						my $menuname = readMenu($menus, \@blocks);
						push(@{$outline}, ["MENU", [$menuname, ""]]);
					}
					elsif($tagidx == 2) { 
						readStringTable($strings, \@blocks);
						push(@{$outline}, ["STRINGTABLE", [@blocks]]);
					}
					elsif($tagidx ==3)
					{
						push(@{$outline},["VERSIONINFO",[@blocks]]);
						push(@$versionInfo, @blocks);
					}
					elsif($tagidx ==4)
					{
						my $dlgname = readDesignInfo($designInfos, \@blocks);
						push(@{$outline},["DESIGNINFO",[$dlgname, ""]]);
					}
					elsif($tagidx < @InTags){
						push(@{$outline},["BLOCK",[@blocks]]);
					}

					$tagidx=0;
					@blocks = ();
					$bInBlock = 0;
				}
			}
		}
		else {
			push(@text, $curline);
			$tagidx=0;
		}
	}
	if(@text) {		#flush last texts to outline
		push(@{$outline}, ["__TEXT__",[@text]]);
		@text=();
	}
}

#--------------------------------------------------------------------------------------------------
sub readDialog {
	my ($dialogs, $input) = @_;

	my $dlgname;
	my $fontname;
	my $linenum = 0;
	my @data = ();
	my %lookup = ();

	foreach(@$input){
		$linenum++;
		if(/(ID\S+)\s+DIALOGEX\b/) {
			$dlgname = $1;
			next;
		}
		if(/\bFONT\b.*(".*")/) {
			$fontname = $1;
			$dialogs->{$dlgname}->{"__FONT__"} = [$linenum, $fontname];
			next;
		}
		if(/^CAPTION\b.*(".*")/) {
			$lookup{"CAPTION"} = $linenum;
			push(@data, [$linenum, $1]);
			next;
		}
		next if /^STYLE\b/;
		next if /^BEGIN\b/;
		next if /^END\b/;

		my $line = skipNonTranslatedStr($_);

		if ($line=~/("[^"](?:[^"]|"")*")/) {
			my $value = $1;
			if($line =~ /(?:,|\s)(ID[^,]*),/) {
				my $id = $1;
				if($id ne "IDC_STATIC") {
					$lookup{$id} = $linenum;
				}
			}
			push(@data, [$linenum, $value]);
		}
	}
	$dialogs->{$dlgname}->{"__TEXT__"}=[@$input];
	$dialogs->{$dlgname}->{"__DATA__"}=[@data];
	$dialogs->{$dlgname}->{"__LINES__"} = $linenum;
	$dialogs->{$dlgname}->{"__ID__"} = [%lookup];

	$dlgname;
}

#--------------------------------------------------------------------------------------------------
sub readMenu {
	my ($menus, $input) = @_;

	my $menuname;
	my $linenum = 0;
	my @data=();
	my %lookup = ();

	foreach(@$input){
		$linenum++;
		next if /\bBEGIN\b/;
		next if /\bEND\b/;
		next if /\bSEPARATOR\b/;

		if(/(ID\S+)\s+MENU\b/) {
			$menuname = $1;
			next;
		}

		if( /^\s+MENUITEM\s+(".+"),\s+(\S+)\s*$/) {
			my($key, $value) = ($2, $1);
			push(@data, [$linenum, $value]);
			$lookup{$key} = $linenum;
		}
		elsif(/\bPOPUP\b\s+(".*")/){
			push(@data, [$linenum, $1]);
		}

	}
	$menus->{$menuname}->{"__DATA__"} = [@data];
	$menus->{$menuname}->{"__TEXT__"} = [@$input];
	$menus->{$menuname}->{"__LINES__"} = $linenum;
	$menus->{$menuname}->{"__ID__"} = [%lookup];
	$menuname;
}

#--------------------------------------------------------------------------------------------------
sub readStringTable {
	my ($strings, $input) = @_;

	my $savekey;

	foreach(@{$input}){
		if(/^\s+(ID\S+)\s+(".+")/){
			my ($key, $value) = ($1, $2);
			$strings->{$key} = $value;
			$savekey=undef;
		}
		elsif (/^\s+(ID\S+)/) {
			$savekey = $1;
			$strings->{$savekey} = "";
		}
		else {
			if($savekey) {
				s/^\s*//;
				$strings->{$savekey}=$_;
				$savekey=undef;
			}
		}
	}
}

#--------------------------------------------------------------------------------------------------
sub readDesignInfo {
	my ($designInfos, $input) = @_;

	my $dlgname;
	my @data=();

	foreach(@$input){
		if(/(ID\S+),\s*DIALOG\b/) {
			$dlgname = $1;
			last;
		}
	}
	$designInfos->{$dlgname}->{"__TEXT__"}=[@$input];
	$designInfos->{$dlgname}->{"__LINES__"} = $#$input + 1;

	$dlgname;
}

#--------------------------------------------------------------------------------------------------
sub skipNonTranslatedStr {
	my $line = shift;

	$line =~ s/"
				(?:
					Static|Button|msctls_updown32|SysListView32|msctls_trackbar32					#
					|msctls_progress32|SysTreeView32|SysTabControl32|SysAnimate32|SysLink			#skip built-in control names
					|MS\sShell\sDlg|MS\sSans\sSerif|MS\sUI\sGothic									#skip dialog font, but maybe should not because 3 asian languages need change this
					|\\000|(LANGUAGE.+)?\\r\\n|\+\/-												#skip \r\n  \000 +- etc
					|<a>http.+<\/a>|http:\/\/														#skip http links
					|Media\sPlayer\sClassic\s-?\sHome\sCinema|mpc-hc|MPC-HC\sTeam					#skip app names
			|Comments|CompanyName|FileDescription|FileVersion|InternalName|VarFileInfo|StringFileInfo|Translation
			|LegalCopyright|OriginalFilename|ProductName|ProductVersion								#skip versioninfo for locale rc not in mplayerc.rc
				|[-&\/\d\s\.:,%]+(Hz)?																#skip any thing like 6.4.0.0, 100%, 23.976Hz
				)
				"//gx;
	$line;
}

#--------------------------------------------------------------------------------------------------
sub Trace {
	my ($var, $name) = (shift, shift);

	#$Data::Dumper::Indent = 0;
	print ">>>>>>>>>>>>>>>>>>>>>>>>> $name >>>>>>>>>>>>>>>>>>>>>>>\n";
	print Dumper($var);
	print "<<<<<<<<<<<<<<<<<<<<<<<<< $name <<<<<<<<<<<<<<<<<<<<<<<\n\n";
}

#--------------------------------------------------------------------------------------------------
sub readFile {
	my ($filename, $withBOM) = @_;

	open(INPUT, "<$filename") || die "Cannot open $filename to read";
	if($withBOM) {
		binmode(INPUT, ":encoding(UTF16-LE)");
	}

	my @lines = <INPUT>;
	close(INPUT);
	@lines;
}

#--------------------------------------------------------------------------------------------------
sub writeFile {
	my ($filename, $data, $withBOM) = @_;

	open(OUTPUT, ">$filename")|| die "Cannot open $filename to write";

	if($withBOM==1) {
		binmode(OUTPUT);
		print OUTPUT chr(0xff);	print OUTPUT chr(0xfe);	#write unicode bom
		binmode(OUTPUT, ":raw:encoding(UTF16-LE)");
	}
	elsif($withBOM==2) {
		binmode(OUTPUT, ":raw:encoding(UTF16-LE)");
	}

	foreach (@$data) {
		print OUTPUT $_, "\r\n";
	}
	close(OUTPUT);
}

#--------------------------------------------------------------------------------------------------
#
# calculate the largest common set for array1 & array2
# TODO: too slow, should use something fast later.
sub lcs {
	my($a1, $a2, $changes) = @_;

	my $idx=0;
	my $cols = @$a1;
	my $rows = @$a2;

	if (($rows==0) or ($cols==0)) { return;}

	#optimize the equal test, convert string compare to integer equal test
	my @leftIdx = ();
	my @rightIdx = ();
	my %stringIdx = ();

	my $idx = 0;
	my $key = 0;
	foreach(@{$a1}) {
		if(not exists $stringIdx{$_}) {
			$stringIdx{$_} = $key++;
		}
		$leftIdx[$idx++] = $stringIdx{$_};
	}

	$idx=0;
	foreach(@{$a2}) {
		if(not exists $stringIdx{$_}) {
			$stringIdx{$_} = $key++;
		}
		$rightIdx[$idx++] = $stringIdx{$_};
	}

	#Trace(\%stringIdx, "string index");

	my ($r, $c, $i);

	my $align=[[0,0],[0,0]];
	for($r=0;$r<=$rows;$r++) {
		for($c=0;$c<=$cols;$c++) {
			$align->[$r][$c]=0;
		}
	}

	for($r=1; $r<=$rows; $r++) {
		for($c=1; $c<=$cols; $c++) {
			if( $leftIdx[$c-1] == $rightIdx[$r-1]) {
				$align->[$r][$c] = $align->[$r-1][$c-1] + 1;
			}
			else {
				$align->[$r][$c] = ($align->[$r-1][$c] >= $align->[$r][$c-1])? $align->[$r-1][$c] : $align->[$r][$c-1];
			}
		}
	}

	$idx=0;
	for($r=$rows, $c=$cols, $i=$align->[$r][$c];
			$i>0 && $r>0 && $c>0;
			$i=$align->[$r][$c]) 
	{
		if($align->[$r-1][$c] == $i) {
			$r--;
		}
		elsif ($align->[$r][$c-1] == $i) {
			$c--;
		}
		elsif ($align->[$r-1][$c-1] == $i-1) {
				$r--;$c--;
				$changes->[$idx++] = [$r,$c];
		}
	}
}

#--------------------------------------------------------------------------------------------------
sub writePatchFile {
	my ($output, $data, $withBOM) = @_;

	my @localData = ();
	foreach (@$data) {
		if($_->[0] eq "DIALOG") {
			my $lines = $_->[1]{"__LINES__"};
			while (my($key, $value)=each(%{$_->[1]})) {
				if($key eq "__LINES__") {
					next;
				}
				else {
					push(@localData, "BEGIN DIALOGEX ".$key." LINES $lines");
					foreach my $pair(@{$value}){
						push(@localData, "$pair->[0]\t\t$pair->[1]");
					}
				}
			}
			push(@localData, "END");
			push(@localData, "");
		}
		elsif($_->[0] eq "STRINGTABLE") {
			my($key, $value)=each(%{$_->[1]});
			push(@localData, "STRING $key\t\t$value");
		}
		elsif($_->[0] eq "MENU") {
			my $lines = $_->[1]{"__LINES__"};
			while (my($key, $value)=each(%{$_->[1]})) {
				if($key eq "__LINES__") {
					next;
				}
				else {
					push(@localData, "BEGIN MENU ".$key. " LINES $lines");
					foreach my $pair(@{$value}){
						push(@localData, "$pair->[0]\t\t$pair->[1]");
					}
				}
			}
			push(@localData, "END");
			push(@localData, "");
		}
	}

	writeFile($output, \@localData, $withBOM);
}

###################################################################################################
