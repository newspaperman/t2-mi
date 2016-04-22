# t2-mi
This is a commandline T2-MI baseband frame extractor.
It will read a MPEG TS containing a T2-MI data stream from STDIN and output the extracted MPEG TS to STDOUT

Usage: 
1)Use a tool like szap-s2 to tune a transponder
2)Find out the Data PID that contains the T2-MI stream
3) Use dvbstream and pipe to t2-mi and pipe to your video-player

For example:
dvbstream -o 4096 | ./t2-mi 4096 1 | vlc -- -
