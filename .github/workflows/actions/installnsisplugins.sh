#!/bin/sh

set -e

#nsis_install=`reg QUERY 'HKLM\SOFTWARE\WOW6432NODE\NSIS' |fgrep '(Default)'|sed -e 's/.*REG_SZ *\([A-Z]\):\(.*\)/\/\l\1\l\2/' -e 's/\\\\/\//g'`

nsis_plugins='/c/Program Files (x86)/NSIS/Plugins/'
nsis_include='/c/Program Files (x86)/NSIS/Include/'

tmpfile=$TMPDIR/plugin_$$.zip

echo === UAC ===
plugin='https://nsis.sourceforge.io/mediawiki/images/8/8f/UAC.zip'
curl "$plugin" > $tmpfile

unzip -d "$nsis_include" -j -o $tmpfile 'UAC.nsh'
unzip -d "$nsis_plugins/x86-ansi" -o -j $tmpfile 'Plugins/x86-ansi/UAC.dll'
unzip -d "$nsis_plugins/x86-unicode" -o -j $tmpfile 'Plugins/x86-unicode/UAC.dll'

echo === nsProcess ===
plugin='http://forums.winamp.com/attachment.php?attachmentid=48936&d=1309248568'
curl -A 'Mozilla/5.0' "$plugin" > $tmpfile

7z e -y -o"$nsis_include" $tmpfile 'Include/nsProcess.nsh'
7z e -y -o"$nsis_plugins/x86-ansi" $tmpfile 'Plugin/nsProcess.dll'
7z e -y -o"$nsis_plugins/x86-unicode" $tmpfile 'Plugin/nsProcessW.dll'

echo === InetC ===
plugin='https://nsis.sourceforge.io/mediawiki/images/c/c9/Inetc.zip'
curl "$plugin" > $tmpfile

unzip -d "$nsis_plugins/x86-ansi" -o -j $tmpfile 'Plugins/x86-ansi/INetC.dll'
unzip -d "$nsis_plugins/x86-unicode" -o -j $tmpfile 'Plugins/x86-unicode/INetC.dll'

echo === NSISpcre ===
plugin='https://nsis.sourceforge.io/mediawiki/images/4/48/NSISpcre.zip'
curl "$plugin" > $tmpfile

unzip -d "$nsis_include" -o -j $tmpfile 'NSISpcre.nsh'
unzip -d "$nsis_plugins/x86-ansi" -o -j $tmpfile 'NSISpcre.dll'

echo === nsisSlideshow ===
plugin='http://wiz0u.free.fr/prog/nsisSlideshow/latest.php'
curl "$plugin" > $tmpfile

unzip -d "$nsis_plugins/x86-ansi" -o -j $tmpfile 'bin/nsisSlideshow.dll'
unzip -d "$nsis_plugins/x86-unicode" -o -j $tmpfile 'bin/nsisSlideshowW.dll'

echo === "Nsisunz (ansi)" ===
plugin='https://nsis.sourceforge.io/mediawiki/images/1/1c/Nsisunz.zip'
curl "$plugin" > $tmpfile

unzip -d "$nsis_plugins/x86-ansi" -o -j $tmpfile 'nsisunz/Release/nsisunz.dll'

echo === "Nsisunz (unicode)" ===
plugin='https://nsis.sourceforge.io/mediawiki/images/5/5a/NSISunzU.zip'
curl "$plugin" > $tmpfile

unzip -d "$nsis_plugins/x86-unicode" -o -j $tmpfile 'NSISunzU/Plugin unicode/nsisunz.dll'

echo === ApplicationID ===
plugin='https://github.com/connectiblutz/NSIS-ApplicationID/releases/download/1.1/NSIS-ApplicationID.zip'
curl -L "$plugin" > $tmpfile

unzip -d "$nsis_plugins/x86-ansi" -o -j $tmpfile 'Release/ApplicationID.dll'
unzip -d "$nsis_plugins/x86-unicode" -o -j $tmpfile 'ReleaseUnicode/ApplicationID.dll'

rm -f $tmpfile
