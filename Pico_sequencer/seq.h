// sequencer related definitions and structures

#define SEQ_STEPS 16 // 16 step sequencer
#define NOTERANGE 12 // notes can be +- one octave from root - display limitation
#define GATERANGE 7  // gate time 0-7 ie 12.5% increments
#define VELOCITYRANGE 32  // velocity has 32 steps ie 2.5% per step. makes spinning the encoder less tedious
#define VELOCITYSCALE 4 // 32*4=128 which we limit to 127 
#define PROBABILITYRANGE 9  // probability 0-9 ie 10% increments
#define RATCHETRANGE 3 // number of ratchets/repeats per step 0-3
#define MODRANGE 127  // modulation range 0-127

// clock related stuff
enum STEPMODE {FORWARD,BACKWARD,PINGPONG,RANDOMWALK,RANDOM};

long clocktimer = 0; // clock rate in ms
long notetimer[NTRACKS]={0,0,0,0}; // note off timer
int16_t active_note[NTRACKS]; // note # note in progress, 0 if no note sounding
int16_t active_velocity[NTRACKS]; // velocity of the active note
int16_t active_notelength[NTRACKS]; //length of the active note in ms
bool tie[NTRACKS];  // flag that a tied note is in progress
int16_t ratchetcnt[NTRACKS]; // number of ratchets for the note 
// const char * textrates[] = {" 8x"," 6x"," 4x"," 3x", " 2x","1.5x"," 1x","/1.5"," /2"," /3"," /4"," /5"," /6"," /7"," /8"," /9"," /10"," /11"," /12"," /13"," /14"," /15"," /16"," /32"," /64"," /128"};
int16_t divtable[] = {3,4,6,8,12,16,24,36,48,72,96,120,144,168,192,216,240,264,288,312,336,360,384,768,1536,3072};

int16_t lastCC[NTRACKS]; // we save the last CC message - reduce MIDI traffic by not sending the same message twice 

// all of the sequences use the same data structure even though the data is somewhat different in each case
// this simplifies the code somewhat
// clocks are setup for 24ppqn MIDI clock
// note that there are two threads of execution running on the two Pico cores - UI and note handling
// must be careful about editing items that are used by the 2nd Pico core for note timing etc

struct sequencer {
  int16_t val[SEQ_STEPS];  // values of note offsets from root, gate lengths etc. 
  int16_t active[SEQ_STEPS];  // step active or not. possible values : 1=active, 0=deactivated, -1=cannot be changed
  int16_t max;    // maximum positive value of val - used for UI scaling
  int16_t index;    // index of step we are on
  int16_t stepmode;    // step mode - fwd, backward etc
  int16_t state;    // state - used for step modes  
  int16_t first;  // first step used - anything used my the menusystem has to be integer type
  int16_t last;   // last step used
  int16_t euclen;   // euclidean length
  int16_t eucbeats;   // euclidean beats
  int16_t divider;   // clock rate divider - lookup via table
  int16_t clockticks;   //  clock counter
  int16_t root;   // "root" note - note offsets are relative to this. also used for euclidean offset and CC number
};

// notes are stored as offsets from the root 
sequencer notes[NTRACKS] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  NOTERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, //  euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  NOTERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, //  euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  NOTERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, //  euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  NOTERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, //  euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note
};

// offsets (translations) are added to the current note
sequencer offsets[NTRACKS] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  NOTERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  NOTERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  NOTERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  NOTERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note
};

sequencer gates[NTRACKS] = {
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  // initial data
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // gates may not be deactivated
  GATERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  // initial data
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // gates may not be deactivated
  GATERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  // initial data
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // gates may not be deactivated
  GATERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,  // initial data
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // gates may not be deactivated
  GATERANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note
};

sequencer ratchets[NTRACKS] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  RATCHETRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  RATCHETRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  RATCHETRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  RATCHETRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note
};

// velocities have MIDI values 0-127 
sequencer velocities[NTRACKS] = {
  22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,  // initial setting ~ 80% velocity
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  VELOCITYRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, //  euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,  // initial setting ~ 80% velocity
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  VELOCITYRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, //  euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,  // initial setting ~ 80% velocity
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  VELOCITYRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, //  euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note

  22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,  // initial setting ~ 80% velocity
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  VELOCITYRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, //  euclidean beats
  6,  // clock divide
  24,    // clock counter
  60,   // root note
};

// probability values 
sequencer probability[NTRACKS] = {
  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,  // initial data 100% probability
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  PROBABILITYRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  0,   // holds euclidean offset in this case

  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  PROBABILITYRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  0,   // holds euclidean offset in this case

  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  PROBABILITYRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  0,   // holds euclidean offset in this case

  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,  // initial data
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  PROBABILITYRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  0,   // holds euclidean offset in this case
};

// modulation values 
sequencer mods[NTRACKS] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // initial data 
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  MODRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  16,   // CC number in this case

  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // initial data 
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  MODRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  17,   // CC number in this case

  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // initial data 
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  MODRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  18,   // CC number in this case

  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  // initial data 
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // all steps active by default
  MODRANGE,  // maximum value
  0,   // step index
  FORWARD, // step mode
  0,     // state - used for step modes
  0,   // first step
  SEQ_STEPS-1,  // last step
  SEQ_STEPS, // euclidean length
  1, // euclidean beats
  6,  // clock divide
  24,    // clock counter
  19,   // CC number in this case
};



// clock a sequencer
// you have to pass a pointer to the sequence structure, not the structure itself
// this is to allow modifying the contents of the structure - baffled me for a while 
// returns 1 when index changes - in the case of gates this is a note on event
int16_t seqclock(sequencer *seq) {
  int16_t event=0;
  --seq->clockticks;
  if (seq->clockticks < 1 ) { // divider has rolled over
    seq->clockticks=divtable[seq->divider];  // lookup table used to get clock divider
    event=1;
    switch (seq->stepmode) {
      case FORWARD:
        ++seq->index;
        if (seq->index > seq->last) seq->index=seq->first;
        break;
      case BACKWARD:
        --seq->index;
        if (seq->index < seq->first) seq->index=seq->last;
        break;
      case PINGPONG:
        if (seq->state == FORWARD) {
         ++seq->index;
          if (seq->index > seq->last) {
            seq->index=seq->last-1;
            seq->index=constrain(seq->index,seq->first,seq->last);
            seq->state=BACKWARD;
          }
        }
        else {
          --seq->index;
          if (seq->index < seq->first) {
            seq->index=seq->first+1;
            seq->index=constrain(seq->index,seq->first,seq->last);
            seq->state=FORWARD;
          }
        }
        break;
        case RANDOMWALK:
          seq->index+=random(-1,2); // range of -1 to +1
          seq->index=constrain(seq->index,seq->first,seq->last);
        break;
        case RANDOM:
          seq->index=random(seq->first,seq->last);
        break;        
      default:
        break;
    }
  }
  return event;
  //Serial.printf("ticks %d stepindex %d \n",seq->clockticks,seq->index);
}

// clock all the sequencers
// clockperiod is the period of the 24ppqn clock - used for calculating gate times etc
// this code got a bit messy after I added multiple tracks
// it loops thru all tracks, all sequences looking for note on and off events to process
void clocktick (long clockperiod) {
  int16_t gatestate,ccval;
  for (uint8_t track=0; track<NTRACKS;++track) {

    // a clock tick has expired so clock the sequencers
    seqclock(&notes[track]);  // have to call by reference
    seqclock(&offsets[track]);
    seqclock(&velocities[track]);
    seqclock(&probability[track]);
    seqclock(&ratchets[track]);
    gatestate=seqclock(&gates[track]);  

    // check if gate became active and if so send note on
    if (gatestate && notes[track].active[notes[track].index] && trackenabled[track] && (probability[track].val[probability[track].index] > random(PROBABILITYRANGE-1))) {
      active_notelength[track]=clockperiod*gates[track].val[gates[track].index]*gates[track].divider/GATERANGE;     // calculate notelength is ms from gate length
      if(ratchets[track].val[ratchets[track].index] >0) ratchetcnt[track]=(ratchets[track].val[ratchets[track].index]+1)*2-1; // for 1 ratchet the count is 3(noteon) 2 (noteoff) 1 (noteon) 0 (noteoff)
      if ((active_notelength[track] > 0) && (ratchetcnt[track] > 0)) { // if we have ratchets divide up the notelength to the number of ratchets
        active_notelength[track]=clockperiod*gates[track].divider/(ratchetcnt[track]+1); // for 1 ratchet (2 notes) divide the note time in four and send noteon/noteoff when the count changes ie 50% gate 
      }
      notetimer[track]=millis()+active_notelength[track];
      if ((active_notelength[track] > 0) && (!tie[track])) {  // no note on when gate is zero or a tied note is in progress
        active_note[track]=notes[track].val[notes[track].index]+offsets[track].val[offsets[track].index]*offsets[track].active[offsets[track].index]+notes[track].root;
        active_note[track] = constrain(active_note[track],0,127); // limit to MIDI range
        active_note[track]= quantize(active_note[track],scales[current_scale[track]],notes[track].root); // quantize to current root and scale
        active_velocity[track]=constrain(velocities[track].val[velocities[track].index]*VELOCITYSCALE,0,127);
        noteOn(MIDIchannel[track]-1,active_note[track],active_velocity[track]);
        //Serial.printf("noteon %d\n",active_note);
      }
      if ((gates[track].val[gates[track].index]==GATERANGE) && (ratchetcnt[track]==0)) tie[track]=TRUE; // 100% gate is a tied note, unless we are ratcheting
      else tie[track]=FALSE;
      //Serial.printf("notelength %d\n",notelength);
    }

    // process note offs and ratchets
    // the code produces 50% gate time on ratchets
    // this was hard to get right! maybe should be a state machine

    if (millis() > notetimer[track]) { // if note timer has expired
      if (active_note[track] && (!tie[track])) {
        if (ratchetcnt[track] >0) { // we are ratcheting
          if (ratchetcnt[track] & 1) noteOff(MIDIchannel[track]-1,active_note[track],0); // ratcheting - note off on odd ratchet counts
        }
        else noteOff(MIDIchannel[track]-1,active_note[track],0); // not ratcheting, turn note off
        if (ratchetcnt[track]==0) active_note[track]=0;  // its the last ratchet
        else notetimer[track]=millis()+active_notelength[track]; // schedule another
      }
      if (ratchetcnt[track] && active_note[track]) {  // we are ratcheting so send another note on
        if (!(ratchetcnt[track] &1)) noteOn(MIDIchannel[track]-1,active_note[track],active_velocity[track]); // send noteon every 2nd count
        //Serial.printf("noteon %d\n",active_note);
        if ((--ratchetcnt[track]) == 0) active_note[track]=0;
      }     
    }

    // process mod sequencers
    gatestate=seqclock(&mods[track]); 
    if (gatestate) { // true when sequencer steps
      ccval=mods[track].val[mods[track].index]; // get the CC value to send
      if ((mod_enabled[track]) && (ccval >=0) && (ccval!=lastCC[track])) { // CC value -1 means don't send anything. don't send same CC message over and over
        controlChange(((byte)CCchannel[track])-1,(byte)mods[track].root,(byte)ccval); // in this case seq.root is the CC number
        lastCC[track]=ccval;
      }
    }
  }
}
 

// must be called regularly for sequencer to run
void do_clocks(void) {
  long clockperiod= (long)(((60.0/(float)bpm)/PPQN)*1000);
  if ((millis() - clocktimer) > clockperiod) {
    clocktimer=millis(); 
    clocktick(clockperiod);
  }
}

// send noteoff for all notes
void all_notes_off(void) {
  for (uint8_t track=0; track<NTRACKS;++track) {
    noteOff(MIDIchannel[track]-1,active_note[track],0); // turn the note off
  }
}

// resets all clock counters and indices to get everything back in sync
void sync_sequencers(void){
  for (int track=0; track<NTRACKS;++track) {
    notes[track].clockticks=divtable[notes[track].divider];  // lookup table used to get clock divider
    notes[track].index=0;
    gates[track].clockticks=divtable[gates[track].divider];;
    gates[track].index=0;
    velocities[track].clockticks=divtable[velocities[track].divider];
    velocities[track].index=0;
    offsets[track].clockticks=divtable[offsets[track].divider];
    offsets[track].index=0;
    probability[track].clockticks=divtable[probability[track].divider];
    probability[track].index=0;
    ratchets[track].clockticks=divtable[ratchets[track].divider];
    ratchets[track].index=0;
  }
}

// Euclidean calculation functions from http://clsound.com/euclideansequenc.html

/*Function to right rotate n by d bits*/

uint16_t rightRotate(int shift, uint16_t value, uint8_t pattern_length) {
  uint16_t mask = ((1 << pattern_length) - 1);
  value &= mask;
  return ((value >> shift) | (value << (pattern_length - shift))) & mask;
}

// Function to find the binary length of a number by counting bitwise
int findlength(unsigned int bnry) {
  boolean lengthfound = false;
  int length = 1; // no number can have a length of zero - single 0 has a length of one, but no 1s for the sytem to count
  for (int q = 32; q >= 0; q--) {
    int r = bitRead(bnry, q);
    if (r == 1 && lengthfound == false) {
      length = q + 1;
      lengthfound = true;
    }
  }
  return length;
}

// Function to concatenate two binary numbers bitwise
unsigned int ConcatBin(unsigned int bina, unsigned int binb) {
  int binb_len = findlength(binb);
  unsigned int sum = (bina << binb_len);
  sum = sum | binb;
  return sum;
}

uint16_t euclid(int n, int k, int o) { // inputs: n=total, k=beats, o = offset
  int pauses = n - k;
  int pulses = k;
  int offset = o;
  int steps = n;
  int per_pulse = pauses / k;
  int remainder = pauses % pulses;
  unsigned int workbeat[n];
  unsigned int outbeat;
  uint16_t outbeat2;
  unsigned int working;
  int workbeat_count = n;
  int a;
  int b;
  int trim_count;

  for (a = 0; a < n; a++) { // Populate workbeat with unsorted pulses and pauses
    if (a < pulses) {
      workbeat[a] = 1;
    }
    else {
      workbeat[a] = 0;
    }
  }

  if (per_pulse > 0 && remainder < 2) { // Handle easy cases where there is no or only one remainer
    for (a = 0; a < pulses; a++) {
      for (b = workbeat_count - 1; b > workbeat_count - per_pulse - 1; b--) {
        workbeat[a] = ConcatBin(workbeat[a], workbeat[b]);
      }
      workbeat_count = workbeat_count - per_pulse;
    }

    outbeat = 0; // Concatenate workbeat into outbeat - according to workbeat_count
    for (a = 0; a < workbeat_count; a++) {
      outbeat = ConcatBin(outbeat, workbeat[a]);
    }

    if (offset > 0) {
      outbeat2 = rightRotate(offset, outbeat, steps); // Add offset to the step pattern
    }
    else {
      outbeat2 = outbeat;
    }

    return outbeat2;
  }

  else {
    if (pulses == 0) {
      pulses = 1;  //	Prevent crashes when k=0 and n goes from 0 to 1
    }
    int groupa = pulses;
    int groupb = pauses;
    int iteration = 0;
    if (groupb <= 1) {
    }

    while (groupb > 1) { //main recursive loop
      /*Serial.print("Pulses: ");		//	Help debug crash
        Serial.println(pulses);
        Serial.print("Pauses: ");
        Serial.println(pauses);
        Serial.print("Iterations: ");
        Serial.println(iteration);*/
      if (groupa > groupb) { // more Group A than Group B
        int a_remainder = groupa - groupb; // what will be left of groupa once groupB is interleaved
        trim_count = 0;
        for (a = 0; a < groupa - a_remainder; a++) { //count through the matching sets of A, ignoring remaindered
          workbeat[a] = ConcatBin(workbeat[a], workbeat[workbeat_count - 1 - a]);
          trim_count++;
        }
        workbeat_count = workbeat_count - trim_count;

        groupa = groupb;
        groupb = a_remainder;
      }

      else if (groupb > groupa) { // More Group B than Group A
        int b_remainder = groupb - groupa; // what will be left of group once group A is interleaved
        trim_count = 0;
        for (a = workbeat_count - 1; a >= groupa + b_remainder; a--) { //count from right back through the Bs
          workbeat[workbeat_count - a - 1] = ConcatBin(workbeat[workbeat_count - a - 1], workbeat[a]);

          trim_count++;
        }
        workbeat_count = workbeat_count - trim_count;
        groupb = b_remainder;
      }

      else if (groupa == groupb) { // groupa = groupb
        trim_count = 0;
        for (a = 0; a < groupa; a++) {
          workbeat[a] = ConcatBin(workbeat[a], workbeat[workbeat_count - 1 - a]);
          trim_count++;
        }
        workbeat_count = workbeat_count - trim_count;
        groupb = 0;
      }

      else {
        //Serial.println("ERROR");
      }
      iteration++;
    }

    outbeat = 0; // Concatenate workbeat into outbeat - according to workbeat_count
    for (a = 0; a < workbeat_count; a++) {
      outbeat = ConcatBin(outbeat, workbeat[a]);
    }

    if (offset > 0) {
      outbeat2 = rightRotate(offset, outbeat, steps); // Add offset to the step pattern
    }
    else {
      outbeat2 = outbeat;
    }

    return outbeat2;
  }
}

//------------------end euclidian math-------------------------



// menu function handler for euclidean probability
// when you change the euclidean length, beats or offset this function is called
// it sets the probability to 100% or 0% based on the euclidean pattern
// effectively the same as turning on and off the gates 
// you can also edit the probabilities for even more variation

void eucprobability(void) {
  uint16_t pattern;
  pattern = euclid(probability[current_track].euclen,probability[current_track].eucbeats,probability[current_track].root); // "root" is used for offset in this case
  rp2040.idleOtherCore(); // stop core 1 while we modify sequencer values
  probability[current_track].last=probability[current_track].euclen-1; // reset the sequence length to the euclidean length set in the menus
  for (int i=0;i<probability[current_track].euclen;++i){  // pattern is MSB first
    if (bitRead(pattern,probability[current_track].euclen-i-1)) probability[current_track].val[i]=PROBABILITYRANGE; // 100% probability
    else probability[current_track].val[i]=0;  // 0% probability, same as gate off
  }
  rp2040.resumeOtherCore();
}
