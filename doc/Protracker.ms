.\" $Id$
.TL
Protracker derivative Player development HOWTO
.AU
Simon Peter <dn.tlp@gmx.net>
.
.NH 1
Introduction

.PP
If you want to create a player for a Protracker-derivative format, the
generic Protracker player
.I \%CmodPlayer
that comes with AdPlug can be of great help. It supports all standard
Protracker features, plus a good set of extensions to cope with most
derivative formats.

.PP
When writing a player for a Protracker-derivative format, it is almost
always better to use and extend the
.I \%CmodPlayer
class, instead of writing a whole new player.

.NH 1
Features

.PP
In addition to handling your data just like the original Protracker would,
AdPlug's generic Protracker player has the following extra features:

.IP \[bu]
Special Arpeggio lists, just like in the SA2 format.
.IP \[bu]
Extendable set of up to 256 note commands.
.IP \[bu]
Up to 16 behaviour changing flags.
.IP \[bu]
Arbitrary numbers of rows, patterns, tracks, instruments and orderlist entries.

.PP
The only current limitation is that it has a maximum 9 voice polyphony,
which isn't really a limitation, since the OPL2 is just 9 voices, anyway.

.NH 1
Defaults

.PP
For historical reasons,
.I \%CmodPlayer
sets defaults to some values that needn't be initialized by you. These
are:

.IP \[bu]
The orderlist is preallocated to 128 entries.
.IP \[bu]
The patterndata is preinitialized to 64 patterns with 9 channels and
64 rows each.
.IP \[bu]
The instruments array is preallocated to 250 instruments.

.PP
These are mostly standard Protracker limits. They stem from the original SA2
defaults, for which this was once the player. Look at the
.I \%CmodPlayer
constructor for info on which variables are involved.

.NH 1
Protracker Loaders

.PP
When you decided to extend the
.I \%CmodPlayer
class, instead of writing a whole new player, you do this by writing a
loader for it. This is done very similarily to writing a unique player
for AdPlug. Thus, reading the
.B "AdPlug Player development HOWTO"
is recommended for this task, too.

.PP
Instead of naming your player class
.I \%CxxxPlayer ,
you should name it
.I \%CxxxLoader
or something appropriate if the name is already taken. You then
publicly inherit the
.I \%CmodPlayer
class and fill the missing methods with code. These are now:

.DS I
.CW
static CPlayer *factory(Copl *newopl);
bool load(istream &f, const char *filename);
float getrefresh();
std::string gettype();
.DE

.PP
Plus maybe some (or all) of the other informational methods, listed in the
.B "Player development HOWTO" .
Refer also to that HOWTO to see what the above methods are about.

.NH 1
Designing your loader

.PP
File validation and initial loading is the same as it would be with
any other player. One speciality is that you have to call
.I \%rewind(0)
whenever you completely loaded a valid file. Don't call it when you
just exit your loader because the file was invalid, etc...

.PP
The Protracker player needs at least an orderlist, the patterns and
the instruments to function. Most of the time, you have to convert
these between your file's ordering and
.I \%CmodPlayer 's
internal ordering. Look in the file protrack.cpp,
.I \%CmodPlayer 's
actual sources, for a list on how these constructs are ordered
internally.

.PP
Also, please look up the
.B Defaults
section in this document, to see if you need to reallocate any of the
defaults.

.PP
There are some variables, that you automatically inherited with your new
loader, that you have to set in order to tell the Protracker player
something about your loaded module. These are the following (default preset
values are printed in brackets after the variable name):

.SH
length (unset):
.PP
The orderlist length. When
.I \%CmodPlayer 's
orderlist pointer equals or is bigger than this length, it is
automatically wrapped around to
.I \%restartpos .

.SH
trackord (0):
.PP
Refer to
.B "Loading Patterns"
below on this one.

.SH
restartpos (unset):
.PP
The restarting position in the orderlist, when
.I \%length
is exceeded.

.SH
activechan (0xffff):
.PP
A flag array, telling the player which channels in every pattern are
actually active and to be played. The ordering of this variable is a
bit awkward, so be careful! It is a 16-bit unsigned short, holding the
activeness of a channel in each of its bits,
.B "starting at the highest order bit" .
It also holds values for 16 channels and you can only use the lower 9.
So, to set channel 0 active, you have to set bit 15 (counting from bit
0) to 1 and so on. You only use the upper 9 bits of this
variable. Setting the lower bits will have no effect.

.SH
initspeed (6):
.PP
Initial protracker-compatible speed setting.

.SH
bpm (unset):
.PP
Initial protracker-compatible bpm (sometimes called tempo) setting.

.SH
flags (Standard):
.PP
Refer to
.B "Special Flags"
below.

.SH
nop (0):
.PP
The number of patterns in your module. You don't need to set this
value. If you leave it at 0,
.I \%CmodPlayer
will automatically determine the number of
.B accessed
patterns (which in no way need to be the same as the actual number of
patterns in your module) from the orderlist, in the
.I \%rewind()
method. The value serves only informational purposes, anyway. It is
not needed for the actual playback. If you think you know better than
.I \%CmodPlayer ,
feel free to set it to something else and
.I \%CmodPlayer
won't touch it anymore and display your value instead.

.NH 1
Loading Patterns

.PP
AdPlug's Protracker player stores the tracks (or channels, as some may call
them), that make up a pattern, in a way that makes it possible for you to
reorder and reuse them in an easy way. This also makes storing patterns in
the classic way a bit awkward.

.PP
If you just want to store your tracks the classic Protracker way
(usually the case), first use the
.I \%CmodPlayer::init_trackord()
method to do an initial setup. Then store your tracks in a sequential
manner, counting from 0, in the
.I \%tracks
array. That is, for the first 9 channels of your first pattern, use
.I \%tracks[0..9] .
The second dimension of this array holds the rows of each channel. For
the next 9 channels, you use
.I \%tracks[10..19] ,
and so on.

.PP
If you want to make use of the reorder/reuse feature of the
.I \%trackord
array, please refer to the sa2.cpp source file. This player utilizes
this method.  Basically, the
.I \%trackord
array tells the player which track out of the
.I \%tracks
array it has to insert for each of the 9 tracks of a pattern. Thus,
the first dimension of this array stands for the corresponding pattern
and the next dimension holds the entries for all the 9 tracks.

.PP
.I \%CmodPlayer
orders its note data, in the
.I \%Tracks
struct, the following way:

.PP
.I \%note
holds the note value. A value of 0 means no note. A value of 127 means
"key off". Values from 1 to 96 are actual notes to be
played. Everything else is ignored. The octaves are encoded with the
actual note values. Thus, notes from 1 to 12 are the 12 halftone-steps
of the first, lowest octave, 13 to 24 are those of the next lowest
octave, and so on. Refer to the source code to see which frequencies
are actually associated with the note values.

.PP
.I \%inst
holds the instrument to be played with this note. Again, a 0 value
means no instrument is associated with this note and the last active
instrument is taken instead. Otherwise, the instrument with the number
.I \%inst
minus 1 is fetched from the
.I \%inst
array (it's 0 based).

.PP
.I \%command
holds the command to be issued with this note. All available commands
are listed in the following table:

.TS H
expand;
lb | lb | lb | lb
n | l | l | l .
Value	Command description	Parameters	Range
_
.TH
0xy	Arpeggio	xy=1st note,2nd note	[0-F]
1xx	Frequency slide up	xx=sliding speed	[0-FF]
2xx	Frequency slide down	xx=sliding speed	[0-FF]
3xx	Tone portamento	xx=sliding speed	[0-FF]
4xy	Vibrato	xx=speed,depth	[0-F]
5xy	Tone portamento & volume slide	xy=vol up|vol down	[0-FF]
6xy	Vibrato & volume slide	xy=vol up|vol down	[0-FF]
7xx	Set tempo	xx=new tempo	[0-FF]
8--	Release sustaining note
9xy	Set carrier/modulator volume	xy=car vol|mod vol	[0-F]
10xy	SA2 volume slide	xy=vol up|vol down	[0-F]
11xx	Position jump	xx=new position	[0-FF]
12xx	Set carr. & mod. volume	xx=new volume	[0-3F]
13xx	Pattern break	xx=new row	[0-FF]
14??	Extended command:
.T&
r | l | l | l .
0y	Set chip tremolo	y=new depth	[0-1]
1y	Set chip vibrato	y=new depth	[0-1]
3y	Retrig note	y=retrig speed	[0-F]
4y	Fine volume slide up	y=vol up	[0-F]
5y	Fine volume slide down	y=vol down	[0-F]
6y	Fine frequency slide up	y=freq up	[0-F]
7y	Fine frequency slide down	y=freq down	[0-F]
.T&
n | l | l | l .
15xx	SA2 set speed	xx=new speed	[0-FF]
16xy	AMD volume slide	xy=vol up|vol down	[0-F]
17xx	Set instrument volume	xx=new volume	[0-3F]
18xx	AMD set speed	xx=new speed	[0-FF]
19xx	RAD set speed	xx=new speed	[0-FF]
20xx	RAD volume slide	xx=vol up/down	[0-FF]
21xx	Set modulator volume	xx=new volume	[0-3F]
22xx	Set carrier volume	xx=new volume	[0-3F]
23xx	Fine frequency slide up	xx=freq up	[0-FF]
24xx	Fine frequency slide down	xx=freq down	[0-FF]
25xy	Set carrier/modulator waveform	xy=carr wav|mod wav	[0-3,F]
26xy	Volume slide	xy=vol up|vol down	[0-F]
27xy	Set chip tremolo/vibrato	xy=tr depth|vb depth	[0-1]
28xy	DTM frequency slide	xy=frames up|down	[0-F]
255--	No operation (NOP)
.TE
.B "Table 1."
Effect commands

.PP
The
.I \%param1
and
.I \%param2
variables hold the command's parameters. These are
command-dependant. Refer to the table above to see what they do with
each of the commands and their value ranges. An
.I xx
in the
.B Parameters
column means that
.I \%param1
and
.I \%param2
form one 2-digit parameter, with
.I param1
being the leftmost decimal. Otherwise,
.I x
refers to
.I \%param1
and
.I y
to
.I \%param2 .

.NH 1
Loading Instruments

.PP
For the instrument data,
.I \%CmodPlayer
stores it in the
.I inst[].data[]
array, in the following way:

.TS H
expand;
nb | lb | lb | lb
n | l | l | n .
Index	Operator	Description	OPL2
_
.TH
0	Channel	Feedback strength / Connection type	0xc0
1	Modulator	Amp Mod / Vib / EG type / Key Scaling / Multiple	0x20
2	Carrier	Amp Mod / Vib / EG type / Key Scaling / Multiple	0x23
3	Modulator	Attack Rate / Decay Rate	0x60
4	Carrier	Attack Rate / Decay Rate	0x63
5	Modulator	Sustain Level / Release Rate	0x80
6	Carrier	Sustain Level / Release Rate	0x83
7	Modulator	Wave Select	0xe0
8	Carrier	Wave Select	0xe3
9	Modulator	Key scaling level / Operator output level	0x40
10	Carrier	Key scaling level / Operator output level	0x43
.TE
.B "Table 2."
Instrument data
.I \%inst[].data[] ) (
values

.PP
There are 3 extensions,
.I \%CmodPlayer
imposes to the standard instrument data. These are also stored within
the
.I \%inst[]
array.

.PP
First, there's the "Special Arpeggio". This is explained in the
.B "Special Arpeggio"
section, below.

.PP
Next we got the
.I \%slide
variable. This is a pre-slide value that is always added to the
frequency of the note, whenever this instrument is to be played.

.PP
Finally, there's the
.I \%misc
variable. This is just the holder for the value of the 0xbd register
(i.e. the "drums'n'misc" register) of the OPL2.

.NH 1
Special Flags

.PP
The
.I \%flags
variable holds special flags to change the behaviour of the
player. These are:

.SH
Standard:
.PP
Act like standard Protracker would. This is the default.

.SH
Decimal:
.PP
Command parameters are decimal values, not hexadecimal. For split parameter
commands (i.e. commands with two parameters, like 0xy), this has no effect,
since decimal values would be from 0 to 9, anyway. But commands that take
both parameters as one value (i.e. like 1xx) now take values from 0 to 99
and handle them properly (i.e. decimal wrap-around is now at 9 to 10 and not
at F to 10).

.SH
Faust:
.PP
Treat the files like
.B "Faust Music Creator"
does. This tracker uses a different volume handling scheme. In standard
Protracker, the volume is computed as follows:
.DS I
.CW
final volume = channel volume = instrument volume
.DE
In
.B "Faust Music Creator" ,
it is done like this:
.DS I
.CW
final volume = (channel volume + instrument volume) / 2
.DE

.SH
NoKeyOn:
.PP
This prevents the OPL2 key off/on toggle on every newly played note. Some
trackers require it that way...

.PP
These flags can be set and unset at any time. To set a flag, just
binary
.B OR
it with the
.I \%flags
variable. Use the
.I "\%enum \%Flags"
data type, defined in the
.I \%CmodPlayer
class for this purpose.

.NH 1
Deconstruction

.PP
Be careful when specifying your own deconstructors! These will generally
override
.I \%CmodPlayer 's
deconstructor, because it is defined virtual. Since
.I \%CmodPlayer 's
own deconstructor normally handles all deconstruction work
automatically, you would have to do this manually now.

.PP
To ease this process, there's a method
.I \%deallocate()
that you inherited with your loader. Call this in your own
deconstructor and it will handle the deconstruction of the
.I \%CmodPlayer -internal
objects automatically, as usual.

.NH 1
Special Arpeggio

.PP
To use the "Special Arpeggio" facility, you have to initalize it first. Use
the
.I \%init_specialarp()
method for this purpose.
.I \%CmodPlayer 's
deconstructor or
.I \%dealloc()
automatically handle the deinit for you.

.PP
The special arpeggio uses the 4 variables
.I arpstart ,
.I arpspeed ,
.I arppos
and
.I arpspdcnt
of the
.I Instrument
struct.

.I
[TODO: actually explain. Refer to sa2.[cpp,h] and the original SA2 docs in
the meantime. The following table summarizes the special commands.]
.R

.TS H
nb | lb
n | l .
Value	Command description
_
.TH
252	Set carr. & mod. volume
253	Release sustaining note
254	Arpeggio loop
255	End of special arpeggio
.TE
.B "Table 3."
"Special Arpeggio" commands
