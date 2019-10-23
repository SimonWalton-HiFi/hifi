#!/bin/sh

set -e

nsis_plugins='/c/Program Files/NSIS/Plugins/'
nsis_include='/c/Program Files/NSIS/Include/'

#nsis_plugins='testunzip/NSIS/Plugins/'
#nsis_include='testunzip/NSIS/Include'

tmpfile=$TMPDIR/plugin_$$.zip

echo UAC
plugins='https://nsis.sourceforge.io/mediawiki/images/8/8f/UAC.zip'
curl $plugins > $tmpfile

unzip -d "$nsis_include" $tmpfile 'UAC.nsh'
unzip -d "$nsis_plugins/x86-ansi" $tmpfile 'Plugins/x86-ansi/UAC.dll'
unzip -d "$nsis_plugins/x86-unicode" $tmpfile 'Plugins/x86-unicode/UAC.dll'
