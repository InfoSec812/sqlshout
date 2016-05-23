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

$index=1;
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
OpenTable() ;
$alphabet = "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z 1 2 3 4 5 6 7 8 9 0 [ : ." ;
$items = explode (" ",$alphabet) ;
$alphalist = "" ;
for ($x=0;$x<count($items);$x++) {
	$alphalist .= "<A HREF=\"/modules.php?name=Music&alpha=$items[$x]\">$items[$x]</A> " ;
}
if (isset($query)) {
	$append = " VALUE=\"$query\"" ;
} else {
	$append = "" ;
}
echo "<DIV ALIGN=\"CENTER\">$alphalist<BR><BR><FORM ACTION=\"/modules.php?name=Music&action=search\" METHOD=POST><INPUT TYPE=TEXT SIZE=20 NAME=query" . $append . "><INPUT TYPE=\"SUBMIT\" VALUE=\"Search\"></FORM></DIV>\n" ;
CloseTable() ;

echo "<BR>\n" ;

if ($db->sql_numrows($result) >= 1) {
	if (isset($artist)) {
		OpenTable();
		$query = "SELECT DISTINCT songidx,artist,title,label,purchaseurl FROM nuke_music_playlist WHERE artist='" . addslashes($artist) . "' AND markfordelete!=1 AND original='1' ORDER BY title" ;
		$resultidx = $db->sql_query($query) ;
		echo "<DIV ALIGN=CENTER>
		        <H2><A HREF=\"" . $_SERVER['HTTP_REFERER'] . "\">^^</A> $artist</H2>
			</DIV>
		      <DIV>\n" ;
		while ($song = $db->sql_fetch_object($resultidx)) {
			echo "<FORM NAME=\"" . urlencode($song->title) . "\" ACTION=\"/rate_music.php\" METHOD=POST>
			<INPUT TYPE=\"HIDDEN\" name=\"method\" value=\"Music\">
			<INPUT TYPE=\"HIDDEN\" name=\"artist\" value=\"$song->artist\">
			<INPUT TYPE=\"HIDDEN\" name=\"title\" value=\"$song->title\">
			<INPUT TYPE=\"HIDDEN\" name=\"songidx\" value=\"$song->songidx\">
			<A HREF=\"/request.php?request=" . urlencode($song->sondidx) . "\">[ REQUEST ] </A>$song->title <BR>
			<SELECT name=\"rate\" onChange=\"submit()\">
			        <OPTION value=\"-2\">-2 Makes me want to commit murder</OPTION>
			        <OPTION value=\"-1\">-1 Hate it</OPTION>
			        <OPTION value=\"0\" SELECTED>0 Indifferent</OPTION>
			        <OPTION value=\"1\">1 Not Bad</OPTION>
			        <OPTION value=\"2\">2 I'd Dance to it</OPTION>
			        <OPTION value=\"3\">3 Rrauugghh!!!</OPTION>
			</SELECT></FORM><BR> <A HREF=\"" . $song->purchaseurl . urlencode($song->artist) . "\">[ BUY IT ] <IMG SRC=\"images/" . $song->label . ".jpg\" BORDER=0></A><BR><HR>\n" ;
		}
		echo "</DIV>\n" ;
		CloseTable();
	} elseif (isset($alpha)) {
		OpenTable();
		echo "<DIV ALIGN=CENTER>
				<H2><A HREF=\"/modules.php?name=Music\">^^</A> Artists Beginning With $alpha</H2>
			</DIV>
			<DIV>\n" ;
		$sql = "SELECT DISTINCT artist FROM nuke_music_playlist WHERE artist LIKE '" . $alpha . "%' AND markfordelete!=1 ORDER BY artist";
		$result = $db->sql_query($sql);
		while ($artist = $db->sql_fetch_object($resultidx)) {
			echo "<A HREF=\"/modules.php?name=Music&artist=" . $artist->artist . "\">$artist->artist</A><BR>\n" ;
		}		
		echo "</DIV>\n" ;
		CloseTable();
	} elseif (isset($action)) {
		OpenTable();
		$query = "SELECT DISTINCT filename,artist,title,label,purchaseurl FROM nuke_music_playlist WHERE (artist LIKE '%" . addslashes($query) . "%' OR title LIKE '%" . addslashes($query) . "%') AND markfordelete!=1 AND original=1 ORDER BY artist,title" ;
		$resultidx = $db->sql_query($query) ;
		$numResults = $db->sql_numrows($resultidx) ;
		echo "<DIV ALIGN=\"CENTER\" STYLE=\"font-size: 14pt; font-weight: bold;\">Search Results</DIV><BR>\n" ;
		echo "<DIV ALIGN=\"CENTER\">$numResults items found.<BR><BR></DIV>\n" ;
		echo "<DIV>\n" ;
		while ($song = $db->sql_fetch_object($resultidx)) {
			echo "<FORM NAME=\"" . urlencode($song->title) . "\" ACTION=\"/rate_music.php\" METHOD=POST>
			<INPUT TYPE=\"HIDDEN\" name=\"method\" value=\"Music\">
			<INPUT TYPE=\"HIDDEN\" name=\"artist\" value=\"$song->artist\">
			<INPUT TYPE=\"HIDDEN\" name=\"title\" value=\"$song->title\">
			<INPUT TYPE=\"HIDDEN\" name=\"songidx\" value=\"$song->songidx\">
			<INPUT TYPE=\"HIDDEN\" name=\"data\" value=\"name=Music&artist=$song->artist\">
			<A HREF=\"/request.php?request=" . urlencode($song->sondidx) . "\">[ REQUEST ] </A>$song->artist - $song->title <BR>
			<SELECT name=\"rate\" onChange=\"submit()\">
			        <OPTION value=\"-2\">-2 Makes me want to commit murder</OPTION>
			        <OPTION value=\"-1\">-1 Hate it</OPTION>
			        <OPTION value=\"0\" SELECTED>0 Indifferent</OPTION>
			        <OPTION value=\"1\">1 Not Bad</OPTION>
			        <OPTION value=\"2\">2 I'd Dance to it</OPTION>
			        <OPTION value=\"3\">3 Rrauugghh!!!</OPTION>
			</SELECT></FORM><BR> 
<A HREF=\"" . $song->purchaseurl . urlencode($song->artist) . "\">[ BUY IT ] <IMG SRC=\"images/" . $song->label . ".jpg\" BORDER=0></A><BR><HR>\n" ;
		}
		echo "</DIV>\n" ;
		CloseTable();
	}		
}
include("footer.php");

?>
