/*
 * rol.h - ROL Player by OPLx (oplx@yahoo.com)
 *
 * Visit:  http://tenacity.hispeed.com/aomit/oplx/
 */
#ifndef H_ROLPLAYER
#define H_ROLPLAYER

#include <vector>

#include "player.h"

class CrolPlayer: public CPlayer
{
public:
    CrolPlayer(Copl *newopl)
        : CPlayer         ( newopl )
          ,rol_header     ( NULL )
          ,mNextTempoEvent( 0 )
          ,mCurrTick      ( 0 )
          ,mTimeOfLastNote( 0 )
          ,mRefresh       ( 18.2f )
    {
    }

    ~CrolPlayer();

    virtual bool  load(istream &f);
    virtual bool  update();
    virtual void  rewind(unsigned int subsong);	 // rewinds to specified subsong
	virtual float getrefresh();			         // returns needed timer refresh rate

    virtual std::string gettype() { return std::string("Adlib Visual Composer"); }

private:
    typedef unsigned short    uint16;
    typedef signed   short    int16;
    typedef signed   long int int32;  
    typedef float             real32;

    typedef struct
    {
        uint16 version_major;
        uint16 version_minor;
        char   unused0[40];
        uint16 ticks_per_beat;
        uint16 beats_per_measure;
        uint16 edit_scale_y;
        uint16 edit_scale_x;
        char   unused1;
        char   mode;
        char   unused2[90];
        char   filler0[38];
        char   filler1[15];
        real32 basic_tempo;
    } SRolHeader;

    typedef struct
    {
        int16  time;
        real32 multiplier;
    } STempoEvent;

    typedef struct
    {
        int16 number;
        int16 duration;
    } SNoteEvent;

    typedef struct
    {
        int16 time;
        char  name[9];
        int16 ins_index;
    } SInstrumentEvent;

    typedef struct
    {
        int16  time;
        real32 multiplier;
    } SVolumeEvent;

    typedef struct
    {
        int16  time;
        real32 variation;
    } SPitchEvent;

    typedef std::vector<SNoteEvent>       TNoteEvents;
    typedef std::vector<SInstrumentEvent> TInstrumentEvents;
    typedef std::vector<SVolumeEvent>     TVolumeEvents;
    typedef std::vector<SPitchEvent>      TPitchEvents;

#define bit_pos( pos ) (1<<pos)

    class CVoiceData
    {
    public:
        enum EEventStatus
        {
            kES_NoteEnd   = bit_pos( 0 ),
            kES_PitchEnd  = bit_pos( 1 ),
            kES_InstrEnd  = bit_pos( 2 ),
            kES_VolumeEnd = bit_pos( 3 ),

            kES_None     = 0
        };

        explicit CVoiceData()
            :   mForceNote           ( true )
               ,mEventStatus         ( kES_None )
               ,current_note         ( 0 )
               ,current_note_duration( 0 )
               ,mNoteDuration        ( 0 )
               ,next_instrument_event( 0 )
               ,next_volume_event    ( 0 )
               ,next_pitch_event     ( 0 )
        {
        }

        void Reset()
        {
            mForceNote            = true;
            mEventStatus          = kES_None;
            current_note          = 0;
            current_note_duration = 0;
            mNoteDuration         = 0;
            next_instrument_event = 0;
            next_volume_event     = 0;
            next_pitch_event      = 0;
        }

        TNoteEvents       note_events;
        TInstrumentEvents instrument_events;
        TVolumeEvents     volume_events;
        TPitchEvents      pitch_events;

        bool              mForceNote : 1;
        int               mEventStatus;
        int               current_note;
        int               current_note_duration;
        int               mNoteDuration;
        int               next_instrument_event;
        int               next_volume_event;
        int               next_pitch_event;
    };

    typedef struct
    {
        uint16 index;
        char   record_used;
        char   name[9];
    } SInstrumentName;

    typedef std::vector<SInstrumentName> TInstrumentNames;

    typedef struct
    {
        char   version_major;
        char   version_minor;
        char   signature[6];
        uint16 number_of_list_entries_used;
        uint16 total_number_of_list_entries;
        int32  abs_offset_of_name_list;
        int32  abs_offset_of_data;

        TInstrumentNames ins_name_list;
    } SBnkHeader;

    typedef struct
    {
        char key_scale_level;
        char freq_multiplier;
        char feed_back;
        char attack_rate;
        char sustain_level;
        char sustaining_sound;
        char decay_rate;
        char release_rate;
        char output_level;
        char amplitude_vibrato;
        char frequency_vibrato;
        char envelope_scaling;
        char fm_type;
    } SFMOperator;

    typedef struct
    {
        char ammulti;
        char ksltl;
        char ardr;  
        char slrr;
        char fbc;
        char waveform;
    } SOPL2Op;

    typedef struct
    {
        char        mode;
        char        voice_number;
        SOPL2Op     modulator;
        SOPL2Op     carrier;
    } SRolInstrument;

    typedef struct
    {
        std::string    name;
        SRolInstrument instrument;
    } SUsedList;

    void load_tempo_events     ( istream &f );
    bool load_voice_data       ( istream &f );
    void load_note_events      ( istream &f, CVoiceData &voice );
    void load_instrument_events( istream &f, CVoiceData &voice,
                                 istream &bnk_file, SBnkHeader const &bnk_header );
    void load_volume_events    ( istream &f, CVoiceData &voice );
    void load_pitch_events     ( istream &f, CVoiceData &voice );

    bool load_bnk_info         ( istream &f, SBnkHeader &header );
    int  load_rol_instrument   ( istream &f, SBnkHeader const &header, std::string &name );
    void read_rol_instrument   ( istream &f, SRolInstrument &ins );
    void read_fm_operator      ( istream &f, SOPL2Op &opl2_op );
    int  get_ins_index( std::string const &name ) const;

    void UpdateVoice( int const voice, CVoiceData &voiceData );
    void SetNote( int const voice, int const note );
    void SetNoteMelodic(  int const voice, int const note  );
    void SetNotePercussive( int const voice, int const note );
    void SetFreq  ( int const voice, int const note, bool const keyOn=false );
    void SetVolume( int const voice, int const volume );
    void send_ins_data_to_chip( int const voice, int const ins_index );
    void send_operator( int const voice, SOPL2Op const &modulator, SOPL2Op const &carrier );

    class StringCompare
    {
    public:
        bool operator()( SInstrumentName const &lhs, SInstrumentName const &rhs ) const
        {
            return keyLess(lhs.name, rhs.name);
        }

        bool operator()( SInstrumentName const &lhs, std::string const &rhs ) const
        {
            return keyLess(lhs.name, rhs.c_str());
        }

        bool operator()( std::string const &lhs, SInstrumentName const &rhs ) const
        {
            return keyLess(lhs.c_str(), rhs.name);
        }
    private:
        bool keyLess( const char *const lhs, const char *const rhs ) const
        {
            return stricmp(lhs, rhs) < 0;
        }
    };

    typedef std::vector<CVoiceData> TVoiceData;

    SRolHeader *rol_header;
    std::vector<STempoEvent>    mTempoEvents;
    TVoiceData                  voice_data;
    std::vector<SUsedList>      ins_list;

    int                         mNextTempoEvent;
    int                         mCurrTick;
    int                         mTimeOfLastNote;
    float                       mRefresh;

    static char         bdRegister;
    static char         bxRegister[9];
    static char         volumeCache[11];
    static int    const kSizeofDataRecord;
    static int    const kMaxTickBeat;
    static int    const kSilenceNote;
    static int    const kNumMelodicVoices;
    static int    const kNumPercussiveVoices;
    static int    const kBassDrumChannel;
    static int    const kSnareDrumChannel;
    static int    const kTomtomChannel;
    static int    const kTomtomFreq;
    static int    const kSnareDrumFreq;
    static uint16 const kNoteTable[12];
};

#endif
