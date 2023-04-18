#include <QDebug>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include "chord.h"
#include "note.h"
#include "common.h"
#include "ui.h"
#include "gamescene.h"

// Consctructors/Destructors & Operators {{{

// Used when all notes of a chord are known
Chord::Chord(int start, int duration, bool toPlay[5]):
  duration(duration), start(start), end(start+duration),
  rushStart(start-TOLERANCE_RUSHING),
  dragStart(start+TOLERANCE_DRAGGING),
  rushRelease( (duration!=0)? rushStart+duration : 0 ),
  dragRelease((duration!=0)? dragStart+duration : 0 ),
  spawnTime(0), // TODO: calculate at what time a note should spawn
  noteNB(countTrue(toPlay,5)) {
  int ttl=0;
  for (int i=0; i<5; i++){
    if (toPlay[i]) {
      // At the same time, create those note rectangles with the right colors
      // Dimensions are defined by the duration of the note
      // msToPx handles durations of 0 and returns the length of a short note
      notes[ttl++] = new Note(i,msToPx(duration));
      // Note: the coordinates are set later using the spawn method
    }
  }
}

// Used when constructing chord from parsing a chartfile (line by line)
Chord::Chord(int fret, int start, int end):
  duration((end==0)?0:end-start),start(start),end(end),
  rushStart(start-TOLERANCE_RUSHING),
  dragStart(start+TOLERANCE_DRAGGING),
  rushRelease((duration!=0)? rushStart+duration : 0 ),
  dragRelease((duration!=0)? dragStart+duration : 0 ),
  noteNB(1){
  notes[0] = new Note(fret,msToPx(duration));
  for (int i=1;i<5;i++){
    notes[i] = NULL;
  }
}

// Copy constructor
Chord::Chord(const Chord& chord):
  duration(chord.duration),start(chord.start),end(chord.end),
  rushStart(chord.rushStart),dragStart(chord.dragStart),
  rushRelease(chord.rushRelease),dragRelease(chord.dragRelease),
  spawnTime(chord.spawnTime),noteNB(chord.noteNB){
  for (int i=0;i<5;i++){
    // Notes are pointers so that copying them takes little time (no deep copy)
    notes[i] = chord.notes[i];
  }
}

// Destructor
// Notes are deleted when they reach offscreen (in the despawn method).
// There is therefore a possibility that notes are deleted twice if not handled.
Chord::~Chord(){
  for (int i=0;i<noteNB;i++){
    if (notes[i]!=NULL) delete notes[i];
  }
}

// As chords are stored in a vector, the assignment operator is necessary
Chord& Chord::operator=(const Chord& other) {
    // Check for self-assignment
    if (this == &other) {
        return *this;
    }
    // Copy the non-constant member variables
    start = other.start;
    end = other.end;
    duration = other.duration;
    rushStart = other.rushStart;
    dragStart = other.dragStart;
    rushRelease = other.rushRelease;
    dragRelease = other.dragRelease;
    spawnTime = other.spawnTime;
    noteNB = other.noteNB;
    // Copy the notes array (using the Note copy constructor)
    // bool toPlay[5]{0};
    for (int i = 0;i<other.noteNB;i++) {
      notes[i] = new Note(other.notes[i]->getFret(),msToPx(duration));
      // toPlay[other.notes[i]->getFret()]=true;
    }
    // for (int i=0; i<5; i++){
      // notes[i] = (toPlay[i])? new Note(i,msToPx(duration)):NULL;
    // }
    // Return a reference to the updated object
    return *this;
}

// }}}

// Methods {{{

// Spawn takes a chord and spawns it slightly above the user's screen.
// Since every note moves in the exact same way, their animations are run
// in parallel. This improves performance and makes sure notes of a chord are
// alway at the same height.
void Chord::spawn(GameScene* scene) const{
  // Get the height of the notes in the chord (it's the same for all)
  double noteH = notes[0]->rect().height();
  const double yDist = scene->height()+OFFSCREEN_NOTE_MARGIN;
  // Time during which the note should move
  const int timeline = pxToMs(noteH+yDist);
  // Group which contains the animations for all the individual notes
  QParallelAnimationGroup* groupAnimation = new QParallelAnimationGroup();
  for (int i=0; i<noteNB; i++){
    // Fret object to the current note
    const Fret* fret = scene->getFret( notes[i]->getFret() );
    // X position of that fret in the scene
    const qreal fretX = fret->mapToScene(fret->rect().topLeft()).x();
    // x position of the current note
    // This is necessary if the note doesn't have the same width as the fret
    const qreal x = fretX + (fret->rect().width()/2) - (notes[i]->rect().width()/2);
    // Sets the note's position for the scene
    notes[i]->setPos(x,-noteH);
    // Configure the animation for that note
    QPropertyAnimation* animation = new QPropertyAnimation(notes[i],"pos");
    // Configure animation parameters (start position, end position, duration)
    animation->setStartValue(notes[i]->pos());
    animation->setEndValue(QPointF(notes[i]->pos().x(), yDist));
    animation->setDuration(timeline);
    // Add the note's animation to the group
    groupAnimation->addAnimation(animation);
    // Add the note to the scene so that it becomes visible
    scene->addItem(notes[i]);
  }
  // Set the chord to self destruct once the animation finishes
  // This happens offscreen. Also, the animation group deletes itself.
  // This is here to prevent memory leaks
  connect(groupAnimation,
          &QParallelAnimationGroup::finished,
          new QObject(), // Object linked to groupAnimation to allow self-desctruct
          [=]() {
            groupAnimation->deleteLater(); // free memory used by groupAnimation
            //despawn(); // delete the Chord alongside all of it's notes
  });
  // Start the animation
  groupAnimation->start();
}

// Since the spawn time of the note depends on the dimensions of the scene
// (notes traverse the scene at a constant rate). They are defined by the song
// starts (as the scene is surely mounted to a view at that time).
void Chord::setSpawnTime(int ms){
  spawnTime=ms;
}

// Called when animation finishes
// Destroys the chord (and it's notes)
void Chord::despawn() const{
  for (int i=0;i<noteNB;i++){
    delete notes[i];
  }
}

// Merge takes a chord and merges it with the current chord.
// This is called by the song's consolidate method so that chords with the same
// start time & duration are merged into one, therefore allowing the score engine
// to handle multi-note chords differently than single note chords.
void Chord::merge(Chord* chord){
  for (int i=0;i<5;i++){
    if (chord->notes[i] != NULL){
      notes[noteNB++] = chord->notes[i];
    }
  }
}

// Useful for debugging prints general info about the chord
// TODO: make this a << operator overload.
void Chord::print(){
  std::string states = "{_____}";
  for (int i=0;i<noteNB;i++){
    states[notes[i]->getFret()+1] = 'X';
  }
  qDebug() << QString::fromStdString(states)
           << " NoteNB:" << noteNB
           << " Start:" << start
           << " End:"   << end
           << " Duration:" << duration
           << " Spawn time:" << spawnTime;
}

// }}}

// Getters {{{

int Chord::getStart(){
  return start;
}

int Chord::getEnd(){
  return end;
}

int Chord::getDuration(){
  return duration;
}

int Chord::getRushStart(){
  return rushStart;
}

int Chord::getNbNotes() {
    return noteNB;
}

int Chord::getSpawnTime() const{
  return spawnTime;
}

std::array<bool,5> Chord::getNotes(){
  std::array<bool,5> noteStates = {false,false,false,false,false};
  for (int i=0;i<noteNB;i++){
    noteStates[notes[i]->getFret()] = true;
  }
  return noteStates;
}

// }}}
