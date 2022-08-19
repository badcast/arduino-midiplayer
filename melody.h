#include "pitches.h"

typedef unsigned int wavet;

const wavet __NOTE_WAIT[]
{
  ::Notes::Note_A5, 50,  ::Notes::Note_C6, 50, ::Notes::Note_E6, 500
};
const wavet __NOTE_REM_CRY[]
{
  ::Notes::Note_C6, ::Notes::Note_E5, ::Notes::Note_A5, ::Notes::Note_E5,
  ::Notes::Note_C6, ::Notes::Note_E5, ::Notes::Note_A5, ::Notes::Note_E5,
  ::Notes::Note_C6, ::Notes::Note_E5, ::Notes::Note_A5, ::Notes::Note_E5,
  ::Notes::Note_C6, ::Notes::Note_E5, ::Notes::Note_A5, ::Notes::Note_E5,

  ::Notes::Note_C6, ::Notes::Note_F5, ::Notes::Note_A5, ::Notes::Note_F5,
  ::Notes::Note_C6, ::Notes::Note_F5, ::Notes::Note_A5, ::Notes::Note_F5,
  ::Notes::Note_C6, ::Notes::Note_F5, ::Notes::Note_A5, ::Notes::Note_F5,
  ::Notes::Note_C6, ::Notes::Note_F5, ::Notes::Note_A5, ::Notes::Note_F5,

  ::Notes::Note_D6, ::Notes::Note_G5, ::Notes::Note_B5, ::Notes::Note_G5,
  ::Notes::Note_D6, ::Notes::Note_G5, ::Notes::Note_B5, ::Notes::Note_G5,
  ::Notes::Note_D6, ::Notes::Note_G5, ::Notes::Note_B5, ::Notes::Note_G5,
  ::Notes::Note_D6, ::Notes::Note_G5, ::Notes::Note_B5, ::Notes::Note_G5,

  ::Notes::Note_E6, ::Notes::Note_G5, ::Notes::Note_C6, ::Notes::Note_G5,
  ::Notes::Note_E6, ::Notes::Note_G5, ::Notes::Note_C6, ::Notes::Note_G5,
  ::Notes::Note_D6, ::Notes::Note_G5, ::Notes::Note_B5, ::Notes::Note_G5,
  ::Notes::Note_D6, ::Notes::Note_G5, ::Notes::Note_B5, ::Notes::Note_G5
};

enum WaveSettings {
  statical_duration = 0,
  has_duration = 1
};

struct Wave
{
  const wavet* buffer;
  uint16_t duration;
  byte settings;

  Wave(const wavet* buffer, byte length, uint16_t duration, WaveSettings flag = ::WaveSettings::statical_duration)
  {
    this->buffer = buffer;
    this->duration = duration;
    this->settings = (length & 0x7F) | (flag << 7);
  }

} 
WaveNextStep(__NOTE_WAIT, sizeof(__NOTE_WAIT) / 2 / 2, 0, ::WaveSettings::has_duration),
WaveRemCry(__NOTE_REM_CRY, sizeof(__NOTE_REM_CRY)/2, 125);

class MidiPlayer
{
  protected:
    enum _CONST {
      shf_loop = 0x1,
      shf_volume = 0xE, // max volume 0 - 7
      shf_state = 0x30  // three state. Stop, Play, Pause
    };
    byte basecfg;
    byte pinOut;
    Wave* wave;
    uint16_t wave_pos;

  public:
    MidiPlayer(byte pinOut);
    MidiPlayer(const MidiPlayer&) = delete;
    MidiPlayer& operator=(const MidiPlayer&) = delete;

    void setMelody(Wave *wave);
    void next(void);
    void prev(void);
    void setLoop(bool loop);
    //from 0 to 7
    void setVolume(byte vol);
    const byte getVolume() const;
    bool isPause(void);
    bool isPlay(void);
    bool isStop(void);

    void play(void);
    void pause(void);
    void stop();

    void update(void);
};



MidiPlayer::MidiPlayer(byte pinOut) {
  memset(this, 0x0, sizeof(*this));
  this->pinOut = pinOut;

  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 10000 Hz increments
  OCR1A = F_CPU / (2 * 10000) - 1;
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1 prescaler
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts

  TCCR0B = (TCCR0B & 0b11111000) | 0x01;
}

void MidiPlayer::setVolume(byte vol) {
  vol = vol > 0x7 ? 0x7 : vol;
  basecfg &= _CONST::shf_volume | vol << 1;
}

const byte MidiPlayer::getVolume() const {

}

inline bool MidiPlayer::isPause() {
  return (basecfg & 0x20) != NULL;
}
inline bool MidiPlayer::isPlay() {
  return (basecfg & 0x10) != NULL;
}
inline bool MidiPlayer::isStop() {
  return (basecfg & _CONST::shf_state) != NULL;
}

void MidiPlayer::play() {
  if (!wave)
    wave = &WaveNextStep;

  basecfg |= 0x10;
  // basecfg |= 16;
}

void MidiPlayer::stop() {
  basecfg &= _CONST::shf_state;
  wave_pos = 0;
}

void MidiPlayer::setLoop(bool loop) {
  basecfg &= (_CONST::shf_loop) | (loop == true);
}

void MidiPlayer::setMelody(Wave *wave) {
  bool lastPlayed = isPlay();
  stop();
  this->wave = wave;
  if (lastPlayed)
    play();
}

void MidiPlayer::update() {
  static uint16_t duration = 0;
  int len;

  if (duration < millis() && isPlay() && wave)
  {

    len = wave->settings & 0x7F;
    if(len == 0)
    {
      stop();
      return;
    }

    if (wave->settings >> 0x7)
      duration = wave->buffer[wave_pos % len] + 1;
    else
      duration = wave->duration;

    tone(pinOut, wave->buffer[wave_pos % len]);

    if (++wave_pos == len) // do next note
      wave_pos = 0;

    duration += millis();
  }
}

ISR(TIMER1_COMPA_vect) {


}
