
// graphics related definitions and functions


// canvas is the part of the screen used for graphics
#ifdef OLED_DISPLAY
#define CANVAS_ORIGIN_X  0 // upper right corner
#define CANVAS_ORIGIN_Y  15 // upper right corner
#define CANVAS_HEIGHT 48 
#define CANVAS_WIDTH 128 
#endif

#ifdef TFT18_DISPLAY
#define CANVAS_ORIGIN_X  0 // upper right corner
#define CANVAS_ORIGIN_Y  24 // upper right corner
#define CANVAS_HEIGHT 80 
#define CANVAS_WIDTH 160 
#endif



// plot a note on the screen
// index = position 0-15
// note_offset = offset from root note - range +-12
void drawnote(int16_t index,int16_t note_offset) {
  int x=CANVAS_ORIGIN_X+(CANVAS_WIDTH/SEQ_STEPS)*index;
  int y=CANVAS_ORIGIN_Y + CANVAS_HEIGHT/2 - note_offset*2; //
  display.drawLine(x,y,x+(CANVAS_WIDTH/SEQ_STEPS), y, WHITE);
#ifdef OLED_DISPLAY
  display.display();
#endif
}

// erase a note on the screen
// index = position 0-15
// note_offset = offset from root note - range +-12
void undrawnote(int16_t index,int16_t note_offset) {
  int x=CANVAS_ORIGIN_X+(CANVAS_WIDTH/SEQ_STEPS)*index;
  int y=CANVAS_ORIGIN_Y + CANVAS_HEIGHT/2 - note_offset*2; //
  display.drawLine(x,y,x+(CANVAS_WIDTH/SEQ_STEPS), y, BLACK);
#ifdef OLED_DISPLAY
  display.display();
#endif
}

// required forward declarations
void undrawindex(int16_t index, int16_t active = 1);
void updateseqlen(sequencer seq);

// draw all the notes in a note sequence
void drawnotes(sequencer p) {
  for (int i=0;i< SEQ_STEPS;++i) {
    drawnote(i,p.val[i]);
    undrawindex(i,p.active[i]);
  }
  updateseqlen(p);
}

// plot a bar on the screen
// index = position 0-15
// val - unscaled height of bar
// max - max value of val - used to scale val to the screen

void drawbar(int16_t index,int16_t val, int16_t max) {
  int x=CANVAS_ORIGIN_X+(CANVAS_WIDTH/SEQ_STEPS)*index;
  int y=map(val,0,max,CANVAS_ORIGIN_Y + CANVAS_HEIGHT,CANVAS_ORIGIN_Y); //
  int height=CANVAS_ORIGIN_Y + CANVAS_HEIGHT-y;
  display.fillRect(x,y,(CANVAS_WIDTH/SEQ_STEPS), height, WHITE);
#ifdef OLED_DISPLAY
  display.display();
#endif
}

void undrawbar(int16_t index,int16_t val, int16_t max) {
  int x=CANVAS_ORIGIN_X+(CANVAS_WIDTH/SEQ_STEPS)*index;
  int y=map(val,0,max,CANVAS_ORIGIN_Y + CANVAS_HEIGHT,CANVAS_ORIGIN_Y);
  int height=CANVAS_ORIGIN_Y + CANVAS_HEIGHT-y;
  display.fillRect(x,y,(CANVAS_WIDTH/SEQ_STEPS), height, BLACK);
#ifdef OLED_DISPLAY
  display.display();
#endif
}

// draw all the notes in a note sequence
void drawbars(sequencer p) {
  for (int i=0;i< SEQ_STEPS;++i) {
    drawbar(i,p.val[i],p.max);
    undrawindex(i,p.active[i]);
  }
  updateseqlen(p);
}

// plot sequence length on the screen
// len = last sequence position 0-15
void drawseqlen(int16_t len) {
  int x=CANVAS_ORIGIN_X+(CANVAS_WIDTH/SEQ_STEPS)*len+7;
  int y=CANVAS_ORIGIN_Y -6; //
  display.drawLine(x,y,x, y+4, WHITE);
#ifdef OLED_DISPLAY
  display.display();
#endif
}

void undrawseqlen(int16_t len) {
  int x=CANVAS_ORIGIN_X+(CANVAS_WIDTH/SEQ_STEPS)*len+7;
  int y=CANVAS_ORIGIN_Y -6; //
  display.drawLine(x,y,x, y+4, BLACK);
#ifdef OLED_DISPLAY
  display.display();
#endif
}

// update the sequence length on the screen (vertical bar)
void updateseqlen(sequencer seq) {
  static int16_t last_seqlen = -1; // tracks the sequence length
  if (seq.last != last_seqlen) { // draw the sequence length marker
    undrawseqlen(last_seqlen);
    drawseqlen(seq.last);
    last_seqlen=seq.last;
  }
}

// plot index on the screen
// index = position 0-15
void drawindex(int16_t index, int16_t active = 1) {
  int x=CANVAS_ORIGIN_X+(CANVAS_WIDTH/SEQ_STEPS)*index+4;
  int y=CANVAS_ORIGIN_Y -4; //
  if (active!=0)
    display.fillCircle(x,y,2, WHITE);
  else
    display.fillCircle(x,y,1, WHITE);
#ifdef OLED_DISPLAY
  display.display();
#endif
}

void undrawindex(int16_t index, int16_t active) {
  int x=CANVAS_ORIGIN_X+(CANVAS_WIDTH/SEQ_STEPS)*index+4;
  int y=CANVAS_ORIGIN_Y -4; //
  if (active!=0) {
    display.fillCircle(x,y,3, WHITE);
    display.fillCircle(x,y,2, BLACK);
  }
  else
    display.fillCircle(x,y,3, BLACK);
#ifdef OLED_DISPLAY
  display.display();
#endif
}

// update the index on the screen - LED emulation
void updateindex(sequencer seq) {
  static int16_t last_index = -1; // tracks the sequencer index
  if (seq.index != last_index) { // draw the index marker
    undrawindex(last_index, seq.active[last_index]);
    drawindex(seq.index, seq.active[seq.index]);
    last_index=seq.index;
  }
}

// draw/undraw all indexes and sequence length line
void drawindexes(sequencer p) {
  for (int i=0;i< SEQ_STEPS;++i) 
    undrawindex(i, p.active[i]);
  updateseqlen(p);
}

// edit a note sequence
// both cores using the same data at the same time can cause strange things to happen
// so we idle core 1 while core 0 is changing sequencer values
// returns 0 or the step that was changed 1-16
int16_t editnotes(sequencer *seq) {
  int16_t encvalue,edited_step;
  edited_step=0;  // 0 means no step changed
  for (int steppos=0; steppos< SEQ_STEPS;++steppos) {  
    if((encvalue=enc[steppos].getValue()) !=0) {
      undrawnote(steppos,seq->val[steppos]);
      rp2040.idleOtherCore();
      seq->val[steppos]=constrain(seq->val[steppos]+encvalue,-seq->max,seq->max); // values can be + or -
      rp2040.resumeOtherCore();
      drawnote(steppos,seq->val[steppos]);
      edited_step=steppos+1; // if value changed return its index +1
    }
    ClickEncoder::Button button = enc[steppos].getButton();
    if (button==ClickEncoder::DoubleClicked) { // set end of sequence with double click
      rp2040.idleOtherCore();
      seq->last=steppos;
      rp2040.resumeOtherCore();
    }
    if (button==ClickEncoder::Clicked) { // activate or deactivate a step with single click
      int16_t active = 1-seq->active[steppos];
      if (seq->active[steppos]>-1) {
        rp2040.idleOtherCore();
        seq->active[steppos] = active;
        rp2040.resumeOtherCore();
        if (seq->index!=steppos)
          undrawindex(steppos, active);
      }
    }
  }
  return edited_step;
}

// edit a bar graph type sequence - gates, velocity etc
// returns 0 or the step that was changed 1-16
int16_t editbars(sequencer *seq) {
  int16_t encvalue,edited_step;
  edited_step=0;
  for (int steppos=0; steppos< SEQ_STEPS;++steppos) {  
    if((encvalue=enc[steppos].getValue()) !=0) {
      undrawbar(steppos,seq->val[steppos],seq->max);
      rp2040.idleOtherCore();
      seq->val[steppos]=constrain(seq->val[steppos]+encvalue,0,seq->max); // values can be 0 to max     
      rp2040.resumeOtherCore();
      drawbar(steppos,seq->val[steppos],seq->max);
      edited_step=steppos+1;
    }
    ClickEncoder::Button button = enc[steppos].getButton();
    if (button==ClickEncoder::DoubleClicked) { // set end of sequence with double click
      rp2040.idleOtherCore();
      seq->last=steppos;
      rp2040.resumeOtherCore();
    }
    if (button==ClickEncoder::Clicked) { // activate or deactivate a step with single click
      int16_t active = 1-seq->active[steppos];
      if (seq->active[steppos]>-1) {
        rp2040.idleOtherCore();
        seq->active[steppos] = active;
        rp2040.resumeOtherCore();
        if (seq->index!=steppos)
          undrawindex(steppos, active);
      }
    }
  }
  return edited_step;
}

void drawheader(String text){
//  display.clearDisplay();
  display.fillScreen(BLACK);
  display.setCursor(0,0);
  display.print(text+" ");
  display.print(current_track+1);
}