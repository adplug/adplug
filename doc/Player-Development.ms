.\" $Id$
.TL
AdPlug Player development HOWTO
.AU
Simon Peter <dn.tlp@gmx.net>
.
.NH 1
Precautions

.PP
Be sure to also read the
.B "AdPlug Hacking HOWTO" .

.PP
Your player normally consists of two files (the .h & .cpp files) and you normally
name them by the file extensions, your player handles. For example, the HSC player
consists of the files hsc.cpp & hsc.h, because it handles .hsc files. This is the
same with your player class name. Thus, the HSC player's class is
called
.I \%ChscPlayer .
If any of these names happens to be already taken for other purposes, just
name your player something else, appropriately.

.PP
player.h contains the abstract player interface. You have to include it in your
player to communicate with AdPlug. It also contains some very helpful structures
for use in your player. You don't need to use the structs, but you have to use
the methods provided by the
.I \%opl
object (declared in opl.h, but automatically included from player.h)
inside the player class to do the OPL I/O and initialization work.

.NH 1
Main Work

.PP
All you have to do now is to inherit the
.I \%CPlayer
class into your own player class and fill the abstract methods with
code. You at least have to fill in the following methods:

.DS I
.CW
bool load(istream &f, const char *filename);	// load file
bool update();					// execute replay code
void rewind(unsigned int subsong);		// rewind to specified subsong
float getrefresh();				// return needed timer refresh rate
std::string gettype();				// return file type
.DE

.PP
The other methods from
.I \%CPlayer
just serve informational purposes (as does
.I \%gettype() ,
but it's required anyway ;) ) for AdPlug's info box and needn't to be
filled. It would be nice if you fill them anyway, if that's reasonable for
your player.

.PP
There's one more public method you have to define in your player class:

.DS I
.CW
static CPlayer *factory(Copl *newopl);	// return a new instance
.DE

.PP
Since it is static, it isn't already virtually defined in the
.I \%CPlayer
class and you have to add it manually. This method should return a
freshly initialized object of your player class.  If any errors
occured (e.g. not enough memory), return 0 instead.

.PP
Return true from your
.I \%load()
method, if the file was loaded successfully, or false if it couldn't
be loaded for any reason (will mostly be wrong file type errors, since
AdPlug tries its files with any replayer, it is linked to). Your
.I \%update()
method will be called with the frequency, you return from your
.I \%getrefresh()
method, in Hz. Return true from
.I \%update()
if your module hasn't ended yet. If it looped or ended, return false
from that point, but play further for any subsequent calls to
.I \%update() .
AdPlug will rewind your player by itself, using the
.I \%rewind()
method, when necessary.

.PP
AdPlug passes the number of the subsong, it wants to play next, to the
.I \%rewind()
method of your player. This can be any value from 0 to the value
returned by
.I \%getsubsongs() .
If you haven't provided your own
.I \%getsubsongs() ,
AdPlug will presume your player doesn't have support for subsongs. In
that case, AdPlug will always
.I \%rewind(0) .
Please ignore any value passed with
.I \%rewind()
that is out of spec for your player. This should virtually never
happen, but who knows...

.PP
After initializing your player, AdPlug normally first calls
.I \%load() ,
and then
.I \%getrefresh()
and
.I \%update()
in a loop until something happens (i.e. the user stops playback or the
song ends).
.I \%rewind()
and all the informational methods can be called anytime in between the
other calls, but of course only after
.I \%load()
has been called.

.PP
You can add your own constructors, destructors and methods to your
player object, as you like. AdPlug won't care in any way.
