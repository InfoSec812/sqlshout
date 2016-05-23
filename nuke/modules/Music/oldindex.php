<?php

/************************************************************************/
/* PHP-NUKE: Web Portal System                                          */
/* ===========================                                          */
/*                                                                      */
/* Copyright (c) 2002 by Francisco Burzi                                */
/* http://phpnuke.org                                                   */
/*                                                                      */
/* This program is free software. You can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2 of the License.       */
/************************************************************************/

if (!eregi("modules.php", $_SERVER['PHP_SELF'])) {
    die ("You can't access this file directly...");
}

require_once("mainfile.php");
$module_name = basename(dirname(__FILE__));
get_lang($module_name);
$pagetitle = "- "._ACTIVETOPICS."";
include("header.php");

if (strpos($REQUEST_URI,"?")) {
	$concat = "&" ;
} else {
	$concat = "?" ;
}
OpenTable();

if ($db->sql_numrows($result) >= 1) {
	if (isset($artist)) {
		$query = "SELECT DISTINCT filename,artist,title FROM nuke_playlist WHERE artist='" . addslashes($artist) . "'" ;
		$resultidx = $db->sql_query($query) ;
		echo "<DIV ALIGN=CENTER>
		        <H2><A HREF=\"" . $_SERVER['HTTP_REFERER'] . "\">^^</A> $artist</H2>
			</DIV>
		      <DIV>\n" ;
		while ($song = $db->sql_fetchobject($resultidx)) {
			$streamname = str_replace("/home/dphillips/Music/","",$song->filename) ;
			echo "<FORM ACTION=\"/rate_music.php\" METHOD=POST>
			<INPUT TYPE=\"HIDDEN\" name=\"method\" value=\"Music\">
			<A HREF=\"http://gothic-hawaii.com:8000/file/$streamname\">[ PLAY ] </A>$song->title 
			<SELECT name=\"rate\" onChange=\"submit()\">
			        <OPTION value=\"-2\">-2 Makes me want to commit murder</OPTION>
			        <OPTION value=\"-1\">-1 Hate it</OPTION>
			        <OPTION value=\"0\" SELECTED>0 Indifferent</OPTION>
			        <OPTION value=\"1\">1 Not Bad</OPTION>
			        <OPTION value=\"2\">2 I'd Dance to it</OPTION>
			        <OPTION value=\"3\">3 Rrauugghh!!!</OPTION>
			</SELECT></FORM><BR>\n" ;
		}
		echo "</DIV>\n" ;
	} elseif (isset($alpha)) {
		echo "<DIV ALIGN=CENTER>
				<H2><A HREF=\"/modules.php?name=Music\">^^</A> Artists Beginning With $alpha</H2>
			</DIV>
			<DIV>\n" ;
		$sql = "SELECT DISTINCT artist FROM nuke_playlist WHERE artist LIKE '" . $alpha . "%' ORDER BY artist";
		$result = $db->sql_query($sql);
		while ($artist = $db->sql_fetchobject($resultidx)) {
			echo "<A HREF=\"" . $_SERVER['REQUEST_URI'] . $concat . "artist=$artist->artist\">$artist->artist</A><BR>\n" ;
		}		
		echo "</DIV>\n" ;
	} else {
		$alphabet = "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z" ;
		$items = explode (" ",$alphabet) ;
		for ($x=0;$x<count($items);$x++) {
			echo "<A HREF=\"" . $_SERVER['REQUEST_URI'] . $concat . "alpha=$items[$x]\">$items[$x]</A> " ;
		}
		echo "<BR>\n" ;
	}
}
CloseTable();
include("footer.php");

?>
