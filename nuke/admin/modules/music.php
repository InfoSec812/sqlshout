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

if (!eregi("admin.php", $_SERVER['PHP_SELF'])) { die ("Access Denied"); }

$result = sql_query("select radminsuper, radminmusic, admlanguage from ".$prefix."_authors where aid='$aid'", $dbi);
list($radminsuper,$admlanguage) = sql_fetch_row($result, $dbi);
if (($radminsuper==1) OR ($radminmusic==1)) {

/*********************************************************/
/* Messages Functions                                    */
/*********************************************************/

function music() {
    global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
    include ("header.php");
    GraphicAdmin();
    OpenTable() ;
    echo "<center><font class=\"title\"><b>Song/Artist Search</b></font></center>";
    CloseTable() ;
    echo "<BR>\n<FORM ACTION=admin.php METHOD=GET>" ;
    OpenTable() ;
    echo "	<TR>
		<TD>
			<INPUT TYPE=HIDDEN NAME=op VALUE=musicsearch>
			Search For: <INPUT TYPE=TEXT NAME=srchtxt SIZE=30><BR>
		</TD>
		<TD>
			<INPUT TYPE=Submit VALUE=Submit><BR>
		</TD>
	</TR>\n" ;
    CloseTable() ;
    echo "</FORM>\n" ;
    echo "<BR>\n" ;
    OpenTable() ;
    echo "<center><font class=\"title\"><b>Playlist Managment</b></font></center>";
    CloseTable() ;
    echo "<BR>\n" ;
    OpenTable() ;
    $resultidx = sql_query("SELECT t0.ratingidx,t1.title,t1.artist FROM nuke_music_queue t0, nuke_music_playlist t1 WHERE t0.song=t1.songidx AND t1.markfordelete!=1 ORDER BY ratingidx",$dbi) ;
    $objects = sql_num_rows($resultidx) ;
    $count = 0 ;
    if ($objects > 0) {
        while (list($songid,$title,$artist) = sql_fetch_row($resultidx,$dbi)) {
	             echo "	<TR>
		<TD>
			$songid. $artist - $title<BR>\n
		</TD>
		<TD>
			<A HREF=\"admin.php?op=delqueued&songid=$songid\"><IMG SRC=\"/images/delete.gif\" BORDER=0></A><BR>
		</TD>
		<TD>\n" ;
	    if ($count>0) {
	         echo "			<A HREF=\"admin.php?op=swapup&songid=$songid\"><IMG SRC=\"/images/swapup.gif\" BORDER=0></A><BR>\n" ;
	    }
	    echo "		</TD>
	</TR>\n" ;
	    $count++ ;
        }
    }
    CloseTable() ;
    echo "<BR>\n" ;
    OpenTable();
    echo "<center><font class=\"title\"><b>"._MUSICADMIN."</b></font></center>";
    CloseTable();
    echo "<br>";
    if ($admlanguage == "") {
    	$admlanguage = $language; /* This to make sure some language is pre-selected */
    }
    OpenTable();
    $resultidx = sql_query("SELECT DISTINCT artist FROM nuke_music_playlist WHERE markfordelete!='1' ORDER BY artist",$dbi) ;

    $count = 0 ;
    if (sql_num_rows($resultidx) > 0) {
        while (list($artist) = sql_fetch_row($resultidx,$dbi)) {
            if ($count%2) {
                $color = "#969696" ;
	    } else {
	        $color = "#000000" ;
            }
	    if ($count==0) {
		    echo "	<TR BGCOLOR=\"$color\">\n" ;
	    }
            echo "	<TD><A HREF=\"admin.php?op=artist&artist=" . urlencode($artist) . "\">$artist</A><BR></TD>\n" ;
	    if ($count==4) {
		    echo "	</TR>\n" ;
		    $count = 0 ;
	    }
	    $count++ ;
        }
	if ($count!=0) {
	        for ($finish=$count;$finish<=4;$finish++) {
			echo "<TD></TD>\n" ;
		}
		echo "</TR>\n" ;
	}
    }

    CloseTable();
    include ("footer.php");
}

function playlist () {
    global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
    include "framehead.php" ;
    OpenTable() ;
    $resultidx = sql_query("SELECT t0.ratingidx,t1.title,t1.artist FROM nuke_music_queue t0, nuke_music_playlist t1 WHERE t0.song=t1.songidx AND markfordelete!=1 ORDER BY ratingidx",$dbi) ;
    $objects = sql_num_rows($resultidx) ;
    $count = 0 ;
    if ($objects > 0) {
        while (list($songid,$title,$artist) = sql_fetch_row($resultidx,$dbi)) {
        	echo "	<TR>
		<TD>
			$songid. " . urldecode($artist) . " - " . urldecode($title) . "
		</TD>
		<TD>
			<A HREF=\"admin.php?op=delqueued&songid=$songid\"><IMG SRC=\"/images/delete.gif\" BORDER=0></A><BR>
		</TD>
		<TD>\n" ;
		echo "			<A HREF=\"admin.php?op=swapup&songid=$songid\"><IMG SRC=\"/images/swapup.gif\" BORDER=0></A><BR>\n" ;
		echo "		</TD>
		<TD>\n" ;
		if (($count!=($objects-1)) && ($count!=0)) {
	        	echo "			<A HREF=\"admin.php?op=swapdown&songid=$songid\"><IMG SRC=\"/images/swapdown.gif\" BORDER=0></A><BR>\n" ;
		}
	    echo "		</TD>
	</TR>\n" ;
	    $count++ ;
        }
    }
    CloseTable() ;
    include "framefoot.php" ;
}

function artist ($artist) {
    global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
    include ("header.php");
    GraphicAdmin();
    OpenTable();
    echo "<center><font class=\"title\"><b>"._MUSICADMIN."</b></font></center>";
    CloseTable();
    echo "<br>";
    if ($admlanguage == "") {
    	$admlanguage = $language; /* This to make sure some language is pre-selected */
    }
    OpenTable();
    $resultidx = sql_query("SELECT DISTINCT filename,artist,title,label,purchaseurl FROM nuke_music_playlist WHERE artist='" . urldecode($artist) . "' AND markfordelete!='1' ORDER BY title",$dbi) ;
    $count=0 ;
    if (sql_num_rows($resultidx) > 0) {
        while (list($sqlfilename,$artist,$title,$label,$purchaseurl) = sql_fetch_row($resultidx,$dbi)) {
	    echo "	<TR>\n" ;
            echo "		<TD>$artist - <A HREF=\"admin.php?op=editsong&filename=" . urlencode($sqlfilename) . "&artist=" . urlencode($artist) . "&title=" . urlencode($title) . "&label=" . urlencode($label) . "&purchaseurl=" . urlencode($purchaseurl) . "\">$title</A> <A HREF=\"admin.php?op=deletesong&filename=" . urlencode($sqlfilename) . "&artist=" . urlencode($artist) . "\">[ DELETE ]</A></TD>\n" ;
	    echo "	</TR>\n" ;
        }
    }
    CloseTable();

    include ("footer.php");
}    	

function musicsearch ($srchtxt) {
    global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
    include ("header.php");
    GraphicAdmin();
    OpenTable();
    echo "<center><font class=\"title\"><b>"._MUSICADMIN."</b></font></center>";
    CloseTable();
    echo "<br>";
    if ($admlanguage == "") {
    	$admlanguage = $language; /* This to make sure some language is pre-selected */
    }
    OpenTable();
    $resultidx = sql_query("SELECT DISTINCT filename,artist,title FROM nuke_music_playlist WHERE (artist LIKE '%" . $srchtxt . "%' OR title LIKE '%" . $srchtxt . "%' OR filename LIKE '/home/dphillips/Music/%" . $srchtxt . "%') AND markfordelete!=1 ORDER BY title",$dbi) ;
    $count=0 ;
    if (sql_num_rows($resultidx) > 0) {
        while (list($sqlfilename,$artist,$title) = sql_fetch_row($resultidx,$dbi)) {
	    echo "	<TR>\n" ;
            echo "		<TD><A HREF=\"admin.php?op=artist&artist=$artist\">$artist</A> - <A HREF=\"admin.php?op=editsong&filename=" . urlencode($sqlfilename) . "&artist=" . urlencode($artist) . "&title=" . urlencode($title) . "\">$title</A> <A HREF=\"admin.php?op=deletesong&filename=" . urlencode($sqlfilename) . "&artist=" . urlencode($artist) . "\">[ DELETE ]</A></TD>\n" ;
	    echo "	</TR>\n" ;
        }
    }
    CloseTable();
    sql_free_result($resultidx) ;
    include ("footer.php");
}    	

function editsong($filename,$artist,$title,$label,$purchaseurl) {
    global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
    include ("header.php");
    GraphicAdmin();
    OpenTable();
    echo "<center><font class=\"title\"><b>"._MUSICADMIN."</b></font></center>";
    CloseTable();
    echo "<br>";
    if ($admlanguage == "") {
    	$admlanguage = $language; /* This to make sure some language is pre-selected */
    }
    echo "<FORM ACTION=\"admin.php\" METHOD=GET>\n" ;
    OpenTable();
    echo "	<TR>
		<TD>
			<INPUT TYPE=HIDDEN NAME=\"op\" VALUE=\"savesong\">
			Filename: <INPUT TYPE=HIDDEN NAME=\"filename\" VALUE=\"" . urldecode($filename) . "\">" . urldecode($filename) . "<BR>
		</TD>
	</TR>
	<TR>
		<TD>
			Artist: <INPUT TYPE=TEXT SIZE=30 NAME=\"artist\" VALUE=\"" . urldecode($artist) . "\"><BR>
		</TD>
	</TR>
	<TR>
		<TD>
			Title: <INPUT TYPE=TEXT SIZE=30 NAME=\"title\" VALUE=\"" . urldecode($title) . "\"><BR>
		</TD>
	</TR>
	<TR>
		<TD>
			Label: <INPUT TYPE=TEXT SIZE=30 NAME=\"label\" VALUE=\"" . urldecode($label) . "\"><BR>
		</TD>
	</TR>
	<TR>
		<TD>
			Purchase URL: <INPUT TYPE=TEXT SIZE=30 NAME=\"purchaseurl\" VALUE=\"" . urldecode($purchaseurl) . "\"><BR>
		</TD>
	</TR>
	<TR>
		<TD>
			<INPUT TYPE=SUBMIT VALUE=Submit><BR>
		</TD>
	</TR>\n" ;
    CloseTable();
    echo "</FORM>\n" ;
    include ("footer.php");
}

function savesong($filename,$title,$artist,$label,$purchaseurl) {
    global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
    $resultidx = sql_query("UPDATE nuke_music_playlist SET title='" . addslashes(urldecode($title)) . "', artist='" . addslashes(urldecode($artist)) . "', label='" . addslashes(urldecode($label)) . "', purchaseurl='" . addslashes(urldecode($purchaseurl)) . "' WHERE filename='" . addslashes(urldecode($filename)) . "'",$dbi) ;

    header ("Location: /admin.php?op=artist&artist=" . urlencode($artist)) ;
}

function deletesong($filename,$title,$artist) {
    global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
    $resultidx = sql_query("UPDATE nuke_music_playlist SET markfordelete=1 WHERE filename='" . addslashes(urldecode($filename)) . "'",$dbi) ;

    header ("Location: /admin.php?op=artist&artist=$artist") ;
}

function delqueued($songid) {
    global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
    $resultidx = sql_query("DELETE FROM nuke_music_queue WHERE ratingidx='$songid'",$dbi) ;

    header ("Location: /admin.php?op=music") ;
}

function swapup($songid) {
	global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
	$firstquery = "SELECT ratingidx,song FROM nuke_music_queue WHERE ratingidx<='$songid' ORDER BY ratingidx DESC LIMIT 2" ;
	$resultidx = sql_query("$firstquery",$dbi) ;
	$count=0;
	while(list($ratingidx,$song)=sql_fetch_row($resultidx)) {
		if ($count==0) {
			$first = $song ;
			$firstidx = $ratingidx ;
		} else {
			$second = $song ;
			$secondidx = $ratingidx ;
		}
		$count++ ;
	}
	sql_free_result($resultidx) ;
	sql_query("UPDATE nuke_music_queue SET song='$second' WHERE ratingidx='$firstidx'", $dbi) ;
	sql_query("UPDATE nuke_music_queue SET song='$first' WHERE ratingidx='$secondidx'", $dbi) ;
	header ("Location: /admin.php?op=music") ;
}

function swapdown($songid) {
	global $admin, $admlanguage, $language, $bgcolor1, $bgcolor2, $prefix, $dbi, $multilingual;
	$firstquery = "SELECT ratingidx,song FROM nuke_music_queue WHERE ratingidx>='$songid' ORDER BY ratingidx LIMIT 2" ;
	$resultidx = sql_query("$firstquery",$dbi) ;
	$count=0;
	while(list($ratingidx,$song)=sql_fetch_row($resultidx)) {
		if ($count==0) {
			$first = $song ;
			$firstidx = $ratingidx ;
		} else {
			$second = $song ;
			$secondidx = $ratingidx ;
		}
		$count++ ;
	}
	sql_free_result($resultidx) ;
	$resultidx = sql_query("UPDATE nuke_music_queue SET song='$second' WHERE ratingidx='$firstidx'", $dbi) ;
	sql_free_result($resultidx) ;
	$resultidx = sql_query("UPDATE nuke_music_queue SET song='$first' WHERE ratingidx='$secondidx'", $dbi) ;
	sql_free_result($resultidx) ;
	header ("Location: /admin.php?op=music") ;
}

switch ($op){

    case "music":
    music();
    break;

    case "artist":
    artist($artist);
    break;

    case "editsong":
    editsong($filename,$artist,$title,$label,$purchaseurl);
    break;

    case "savesong":
    savesong($filename,$title,$artist,$label,$purchaseurl) ;
    break ;

    case "deletesong":
    deletesong($filename,$title,$artist);
    break;

    case "delqueued":
    delqueued($songid);
    break;

    case "swapup":
    swapup($songid);
    break;

    case "swapdown":
    swapup($songid);
    break;

    case "musicsearch":
    musicsearch($srchtxt);
    break;

    case "playlist":
    musicsearch($srchtxt);
    break;
}

} else {
    echo "Access Denied";
}

?>
