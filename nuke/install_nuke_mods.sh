#!/bin/bash
echo -n "Enter the name of your PHPnuke database: "
read NUKEDB
echo -n "Enter the hostname/IP of your MySQL database: "
read SQLHOST
echo -n "Enter the port that your MySQL server listens on (Usually 3306): "
read SQLPORT
echo -n "Enter the username for your MySQL Database: "
read SQLUSER
echo -n "Enter the password to log in to your MySQL database: "
read SQLPASS
echo -n "Enter the hostname/IP of of your Icecast/ShoutCast server: "
read SHOUTHOST
echo -n "Enter the base port for your Shout/Ice Cast server: "
read SHOUTPORT
echo -n "Enter the username to log in to the streaming server: "
read SHOUTUSER
echo -n "Enter the password to log in to your streaming server: "
read SHOUTPASS
echo -n "Enter the mount point for the stream (Set to "/" unless you know better): "
read MOUNTPOINT
echo -n "Enter a short name for your stream: "
read SHOUTNAME
echo -n "Enter the stream URL (Associated web site): "
read STREAMURL
echo -n "Enter the stream description: "
read STREAMDESC
echo "Enter the streaming protocol:" 
echo "IceCast Server: 	HTTP"
echo "ShoutCast Server: ICY"
echo -n "Selection: "
read PROTOCOL
echo -n "How many songs before we are allowed to repeat a track? "
read NOREPEAT
echo -n "Run the program in the background? "
read BACKGROUND
echo -n "Enter the bitrate you wish to stream at in Bits/Sec: "
read BITRATE
echo -n "Enter the sample rate you with to stream at in Hz: "
read SAMPLERATE
echo "Enter the number of channels to reencode to:"
echo "0 = Stereo"
echo "1 = Joint Stereo"
echo "3 = Monaural"
echo -n "Selection: "
read CHANNELS
echo -n "Enter the LAME encoder quality from 1 [best-high CPU] to 10 [worst - low CPU]: "
read QUALITY
echo -n "Enter the path to the root of your PHPNuke installation: "
read WEBROOT

#mkdir -p "${WEBROOT}/modules/Music"
#cp modules/Music/* "${WEBROOT}/modules/Music/"
#mkdir -p "${WEBROOT}/admin/modules"
#cp admin/modules/music.php "${WEBROOT}/admin/modules/"

touch sqlshout.conf
echo "DBHOST=${SQLHOST}" >> sqlshout.conf
echo "DBPORT=${SQLPORT}" >> sqlshout.conf
echo "DBUSER=${SQLUSER}" >> sqlshout.conf
echo "DBPASS=${SQLPASS}" >> sqlshout.conf
echo "DBNAME=${NUKEDB}" >> sqlshout.conf
echo "SHOUTHOST=${SHOUTHOST}" >> sqlshout.conf
echo "SHOUTPORT=${SHOUTPORT}" >> sqlshout.conf
echo "SHOUTUSER=${SHOUTUSER}" >> sqlshout.conf
echo "SHOUTPASS=${SHOUTPASS}" >> sqlshout.conf
echo "SHOUTNAME=${SHOUTNAME}" >> sqlshout.conf
echo "PROTOCOL=${PROTOCOL}" >> sqlshout.conf
echo "MOUNTPOINT=${MOUNTPOINT}" >> sqlshout.conf
echo "STREAMDESC=${STREAMDESC}" >> sqlshout.conf
echo "STREAMURL=${STREAMURL}" >> sqlshout.conf
echo "NOREPEAT=${NOREPEAT}" >> sqlshout.conf
echo "BACKGROUND=${BACKGROUND}" >> sqlshout.conf
echo "MPEGMODE=1" >> sqlshout.conf
echo "QUALITY=${QUALITY}" >> sqlshout.conf
echo "BITRATE=${BITRATE}" >> sqlshout.conf
echo "SAMPLERATE=${SAMPLERATE}" >> sqlshout.conf
echo "CHANNELS=${CHANNELS}" >> sqlshout.conf

echo "You will now be prompted for your MySQL password: "
mysql -u ${SQLUSER} -p ${NUKEDB} < sqldefs.mysql

echo -n "Press ENTER to continue"
read A
clear

echo "If you have already made and installed the SQLshout binary and have IceCast/ShoutCast running,"
echo "then you can go ahead and try it out by typing 'sqlshout'"
echo 
echo "You can place the sqlshout.conf file anywhere, but by default sqlshout looks for the conf file"
echo "in the current working directory. To specify a different location for the conf file just"
echo "provide the path after the sqlshout command. (i.e. sqlshout /path/to/sqlshout.conf)."
echo
echo "Done configuring the PHPNuke portions!!"
echo "Now, to import your music, make sure that you have the php binary in /usr/bin/ or change the "
echo "top of loadPlayList script to point to your php-cgi binary. Then, run the program like this:"
echo
echo "  ./loadPlayList /path/to/incoming/MP3/files/ /path/to/store/MP3/files/  "
echo
echo "For my configuration here it like this: loadPlayList /home/dphillips/Music/incoming /home/dphillips/Music"
echo
echo "The script will attempt to use mp3info to determine the artist/title, move the file to the output"
echo "directory, and add entries to the PHPNuke database for each song."
