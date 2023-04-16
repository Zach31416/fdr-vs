#include <fstream>
#include <string>
#include <QString>
#include <QDebug>
#include <regex>
#include <QMediaPlayer>
#include <QGraphicsView>
#include <QObject>
#include <QThread>

#include "song.h"
#include "ui.h"
#include "leftbar.h"
#include "common.h"
#include "timestamp.h"
#include "endmenu.h"

Song::Song(QString chartfile) : chartfile(chartfile) {
    for (int i = 0; i < 4; i++) {
        availableDifficulty[i] = false;
    }
    scene = NULL;
    // mediaPlayer->setNotifyInterval(1);
    parseInfo();
    printInfo();
}

Song::~Song() {
    if (mediaPlayer != NULL) delete mediaPlayer;
}

void Song::parseInfo() {
    std::string stdAudio = chartfile.toStdString();
    std::size_t lastSlash = stdAudio.find_last_of("\\");
    stdAudio = stdAudio.substr(0, lastSlash + 1) + "song.wav";
    audiofile = QString::fromStdString(stdAudio);
    mediaPlayer = new QMediaPlayer();
    mediaPlayer->setMedia(QUrl::fromLocalFile(audiofile));
    std::string tokens[5] = {
      "Name", "Artist", "Album",
      "Year", "Charter"
    };
    QString* values[5] = {
      &title, &artist, &album,
      &year, &charter
    };
    QString boilerplate[5] = {
      "Untitled", "Unknown Artist", "Unknown Album",
      "Year Unknown", "Unknown Charter"
    };
    // True while we are inside the "[Song]" part of the file
    bool insideSong = false;
    std::ifstream file(chartfile.toStdString());
    if (!file.is_open()) {
        qDebug() << "Error opening file";
        return;
    }
    // Create a string to read line-by-line
    std::string line;
    // Search for the beggining of the "[Song" section
    while (getline(file, line)) {
        insideSong = line.find("[Song]");
        // Once this section is reached, start parsing by looking for tokens
        if (insideSong) {
            while (getline(file, line)) {
                // For every token, check if this line contains it
                for (int i = 0; i < 5; i++) {
                    if (line.find(tokens[i]) != std::string::npos) {
                        // There's a match, let's now parse the value from the quotes
                        std::size_t start = line.find("\"");
                        std::size_t end = line.find("\"", start + 1);
                        // get the text between start & end
                        *values[i] = QString::fromStdString(line.substr(start + 1, end - start - 1));
                        // A token was found, no need match another on the same line
                        break;
                    }
                }
                // Resolution isn't inside quotes and must be assigned to an int
                // It is handled differently from the rest here:
                if (line.find("Resolution") != std::string::npos) {
                    resolution = std::stoi(line.substr(line.find_last_of(' ')));
                }
                // Year values often have a comma in them. Let's fix that
                if (year.size() > 4) {
                    std::string yearStr = year.toStdString();
                    yearStr = yearStr.substr(yearStr.find(' ') + 1);
                    year = QString::fromStdString(yearStr);
                }
                // Stop parsing when we reach the end of the "[Song]" section:
                if (line.find("[SyncTrack]") != std::string::npos) break;
            }
        }
    }
    file.close();
}

void Song::parseSync() {
    // We want to prevent offsets in timings.
    // Timeline will store timings in nanoseconds and the different chords
    // will round down to the lower millisecond.
    long int currentTime = 0; // ns
    // Open the chart file
    std::ifstream file(chartfile.toStdString());
    if (!file.is_open()) {
        qDebug() << "There was a problem opening the file";
        return;
    }
    std::string line;
    while (getline(file, line)) {
        // Search for the synctrack section:
        if (line.find("[SyncTrack]") != std::string::npos)
            while (getline(file, line)) {
                std::regex pattern("([0-9]+) = B ([0-9]+)");
                std::smatch match;
                if (std::regex_search(line, match, pattern)) {
                    // We found a match, let's parse the integers
                    int tick = std::stoi(match[1]);
                    int nbpm = std::stoi(match[2]);
                    if (tick == 0) {
                        // First timestamp, we need to add a default one
                        timestamps.push_back(Timestamp(0, tick, nbpm));
                    }
                    else {
                        // We can calculate the time passed since the last timestamp
                        // and add it to the current time and create a new timestamp
                        int tickDiff = tick - timestamps.back().getTick();
                        long int timeDiff = nspt(nbpm, resolution) * tickDiff;
                        currentTime += timeDiff;
                        timestamps.push_back(Timestamp(currentTime, tick, nbpm));
                    }
                }
                if (line.find("[Events]") != std::string::npos) {
                    file.close();
                    return;
                }
            }
    }
    file.close();
    return;
}

bool Song::parseDifficulty(int difficulty) {
    // The token and chosen vector are modular.
    // They adapt to the input difficulty
    std::vector<Chord>* allDiffs[4] = {
      &easy, &medium, &hard, &expert
    };
    std::vector<Chord>* chords = allDiffs[difficulty]; // Vector to use
    std::string allTokens[4] = {
      "EasySingle", "MediumSingle", "HardSingle", "ExpertSingle"
    };
    std::string token = allTokens[difficulty];          // Pattern to match to start parsing
    std::ifstream file(chartfile.toStdString());
    if (!file.is_open()) {
        qDebug() << "Error opening file";
        return false;
    }
    std::string line;
    while (getline(file, line)) {
        // If we find a pattern that matches the token for the desired difficulty
        if (line.find(token) != std::string::npos)
            while (getline(file, line)) {
                // Notes in chords have this format: ` tick = N fret duration`
                // We want to match the tick, fret and duration using regex
                std::regex pattern("([0-9]+) = N ([0-4]+) ([0-9]+)");
                std::smatch match;
                if (std::regex_search(line, match, pattern)) {
                    // We found a match, let's parse the integers
                    int tick = std::stoi(match[1]);
                    int fret = std::stoi(match[2]);
                    int duration = std::stoi(match[3]);
                    // We need to find the timestamp with the closest tick that's smaller
                    int timestampIndex = 0; // NOTE: This could be optimized by starting where the previous chord left off
                    for (uint i = 0; i < timestamps.size(); i++) {
                        if (timestamps[i].getTick() < tick) timestampIndex = i;
                        else break;
                    }
                    // Get the tick of that timestamp
                    const int tsTick = timestamps[timestampIndex].getTick();
                    // Get the current bpm at that timestamp
                    const int tsBPM = timestamps[timestampIndex].getNbpm();
                    // Get the time of that timestamp (in nanoseconds)
                    const long int tsTime = timestamps[timestampIndex].getTime();
                    // Calculate the start time of the chord
                    const long int chordTime = tsTime + (nspt(tsBPM, resolution) * (tick - tsTick));
                    int chordEnd = 0;
                    if (duration != 0) {
                        // Calculate the duration of the chord
                        const long int chordDuration = nspt(tsBPM, resolution) * duration;
                        chordEnd = chordTime + chordDuration;
                    }
                    chords->push_back(Chord(fret, chordTime / 1000, chordEnd / 1000));
                }
                else if (line.find("[") != std::string::npos) {
                    file.close();
                    consolidate(difficulty);
                    availableDifficulty[difficulty] = true;
                    return true;
                }
            }
    }
    // Reaching this point implies the section was found but there were no notes
    return false;
}

void Song::consolidate(int difficulty) {
    std::vector<Chord>* allDiffs[4] = {
      &easy, &medium, &hard, &expert
    };
    std::vector<Chord>* chords = allDiffs[difficulty];
    int chordSize = chords->size();
    for (int i = 0; i < chordSize; i++) {
        Chord* cChord = &(chords->at(i));
        for (int j = i + 1; j < chordSize; j++) {
            Chord* nextChord = &(chords->at(j));
            if ((cChord->getStart() == nextChord->getStart()) &&
                (cChord->getEnd() == nextChord->getEnd())) {
                cChord->merge(nextChord);
                chords->erase(chords->begin() + j);
                j--;
                chordSize--;
            }
        }
    }
    //for (int i = 0; i < chords->size(); i++)
    //{
    //    Chord chordIndividuelle = chords->at(i);
    //    QString tmp = "{";
    //    for (i < 0; i < 5; i++) {
    //        Note* bleh = chordIndividuelle.notes[i];
    //        if (bleh != nullptr)
    //        {
    //            tmp += QString::number(bleh->getFret()) + ", ";
    //            //tmp += (allNotes[i]) ? 'X' : ' ';
    //        }
    //    }
    //    qDebug() << tmp;
    //}
    chords->shrink_to_fit();
}

void Song::printInfo() {
    qDebug() << "----- " << title << ": -----"
        << "\n  Written by: " << artist
        << "\n  In album: " << album
        << "\n  Year: " << year
        << "\n  Charted by: " << charter
        << "\n  Chartfile resolution: " << resolution;
}

void Song::printTimestamps() {
    for (uint i = 0; i < timestamps.size(); i++) {
        qDebug() << " Tick: " << timestamps[i].getTick()
            << " BPM: " << timestamps[i].getBPM()
            << " Time: " << timestamps[i].getTime() << "ns";
    }
    if (timestamps.size() == 0) qDebug() << "No timestamps found\n";
}

void Song::printDifficulty(int difficulty) {
    std::vector<Chord>* chords;
    switch (difficulty) {
    case DIFFICULTY_EASY:
        qDebug() << "----- EasySingle -----";
        chords = &easy;
        break;
    case DIFFICULTY_MEDIUM:
        qDebug() << "----- MediuSingle -----";
        chords = &medium;
        break;
    case DIFFICULTY_HARD:
        qDebug() << "----- HardSingle -----";
        chords = &hard;
        break;
    case DIFFICULTY_EXPERT:
        qDebug() << "----- ExpertSingle -----";
        chords = &expert;
        break;
    default:
        qDebug() << "Requested difficulty is invalid";
        return;
    }
    for (uint i = 0; i < chords->size(); i++) {
        (*chords)[i].print();
    }
}

void Song::play(int difficulty){
  LeftBar* leftbar = scene->getLeftBar();
  leftbar->setTitle(title);
  leftbar->setAlbum(album);
  leftbar->setAuthor(artist);
  // leftbar->setYear(year);
  // leftbar->setYear(mediaPlayer->LoadedState());
  leftbar->setDifficulty(difficulty);
  // Select the correct difficulty
  std::vector<Chord>* allDiff[4] = {
    &easy, &medium, &hard, &expert
  };
  // Fill the Chord buffer
  currentDifficulty = allDiff[difficulty];
  setSpawnTimings(difficulty);
  // Start at the 0th note:
  qDebug() << "INITIATING SONG:";
  currentSpawnChord=0;
  currentScoreChord=0;
  highscore=0;
  bargraphState = 0;
  currentStreak = 0;
  maxStreak = 0;
  currentMultiplier = 1;
  streakReady = false;
  correctlyPlayedNotes = 0;
  activePowerup = false;
  activeDivineIntervention = false;
  // for (int i=0;i<5;i++) scoreTab[i] =0;
  // Initialise a timer that checks muons every second
  muonClock = new QTimer(this);
  muonClock->setInterval(1000);
  connect(muonClock, &QTimer::timeout,this,[=](){
      if (!activeDivineCheck) {
        if ( (muonNb%100) == 0 ){
          divineStart();
        }
      }
      })
  // Initialise the checking timer
  clock = new QTimer(this);
  connect(clock, &QTimer::timeout,this,&Song::spawnHandler);
  connect(clock, &QTimer::timeout,this,&Song::scoreHandler);
  connect(mediaPlayer,
      &QMediaPlayer::mediaStatusChanged,
      this,
      &Song::handleMediaStatusChanged);
  clock->start(1);
  mediaPlayer->setVolume(50);
  leftbar->setDuration(mediaPlayer->duration());
  mediaPlayer->play();
}

// Checks if a note is due to get spawns (is run every ms)
void Song::spawnHandler() {
    // If the last chord to spawn has already been reached, cancel this method
    if (currentSpawnChord + 1 >= currentDifficulty->size()) return;
    if (qint64(currentDifficulty->at(currentSpawnChord).getSpawnTime()) <= mediaPlayer->position()) {
        currentDifficulty->at(currentSpawnChord++).spawn(scene);
    }
}

void Song::divineStart(){
  currentMultiplier *= STREAK_MULT_VALUE;
  activeDivineIntervention = true;
  activeDivineCheck = false; // Stop checking for muons until cooldown end
  // Start a 10s timer that will end the divine intervention
  QTimer* divineTimer = new QTimer();
  divineTimer->setInterval(STREAK_MULT_DURATION);
  divineTimer->setSingleShot(true);
  connect(divineTimer, &QTimer::timeout, this, [=]() {
      divineEnd(divineTimer);
      });
  divineTimer->start();
}

void Song::divineEnd(QTimer* clock){
  // delete the divine intervention clock to prevent memory leaks
  delete clock;
  // Reset the divine intervention
  activeDivineIntervention = false;
  currentMultiplier /= STREAK_MULT_VALUE;
  // Start the cooldown clock
  cooldownClock = new QTimer();
  cooldownClock->setInterval(3000);
  cooldownClock->setSingleShot(true);
  connect(cooldownClock, &QTimer::timeout, this, [=]() {
      divineCooldownEnd(cooldownClock);
      });
  cooldownClock->start();
}

void Song::divineCooldownEnd(QTimer* clock){
  // delete the cooldown clock to prevent memory leaks
  delete clock;
  // Reset the cooldown clock
  activeDivineCheck = true;
}

// Executes when a streak multiplier is activated:
void Song::shake() {
    if (streakReady) {
        activePowerup = true;
        QTimer* onStreak = new QTimer(); // Create a timer
        onStreak->setInterval(STREAK_MULT_DURATION); // set it's duration
        onStreak->setSingleShot(true); // set it to only fire once
        connect(onStreak, &QTimer::timeout, this, [=]() {
            shakeEnd(onStreak); // connect the shakeEnd method to the end of the timer
            });
        streakReady = false; // Reset the readiness of the streak
        bargraphState = 0;
        currentMultiplier *= STREAK_MULT_VALUE; // Update the current score multiplier
        scene->getRightBar()->recolorActivePowerup();
        scene->getRightBar()->setMultiplier(currentMultiplier, activePowerup, activeDivineIntervention);
        // TODO: Change the color of the onscreen bar to blue
        onStreak->start();
    }
}

// Executes when a streak multiplier finishes:
void Song::shakeEnd(QTimer* clock) {
    currentMultiplier /= STREAK_MULT_VALUE; // Revert the multiplier value
    // set the streakMeter int of the bargraph to zero
    // TODO: set the rectangle size to zero
    // TODO: reset the rectangle color to red
    scene->getRightBar()->resetBargraph();
    activePowerup = false;
    streakReady = false;
    scene->getRightBar()->setMultiplier(currentMultiplier, activePowerup, activeDivineIntervention);
    delete clock; // Delete the timer which is no longer needed
}

// Updates the scoring system for each note
void Song::scoreHandler() {
    // If the last chord to hit has already been reached, cancel this method
    if (currentScoreChord + 1 >= currentDifficulty->size()) return;
    /*if (qint64(currentDifficulty->at(currentScoreChord).getRushStart()) < mediaPlayer.position()){
      longNote = (currentDifficulty->at(currentScoreChord).getDuration() !=0 );
      currentScoreChord++;
    }*/
    if (qint64(currentDifficulty->at(currentScoreChord + 1).getRushStart()) < mediaPlayer->position()) {
        currentScoreChord++;
        currentStreak = 0;
        scene->getRightBar()->setStreak(currentStreak);
        //qDebug() << "CurrentChord Now : " + QString::number(currentScoreChord) + " at " + QString::number(mediaPlayer.position());
    }
}

void Song::setSpawnTimings(int difficulty) {
    if (scene == NULL) {
        qDebug() << "Setting spawn timing requires to know the scene"
            << "and it's dimensions";
        return;
    }
    const int totalPx = (FRET_HEIGHT / 2) + scene->getFret(0)->pos().y();
    // const int travelTime = pxToMs(totalPx)+530;
    const int travelTime = pxToMs(totalPx);
    std::vector<Chord>* allDiffs[4] = {
      &easy, &medium, &hard, &expert
    };
    std::vector<Chord>* chords = allDiffs[difficulty];
    for (uint i = 0; i < chords->size(); i++) {
        chords->at(i).setSpawnTime(chords->at(i).getStart() - travelTime);
    }

}

std::vector<Chord>* Song::getChords(int difficulty) {
    std::vector<Chord>* allDiffs[4] = {
      &easy, &medium, &hard, &expert
    };
    std::vector<Chord>* chords = allDiffs[difficulty];
    return chords;
}

void Song::setScene(GameScene* newScene) {
    scene = newScene;
    scene->setSong(this);
}

void Song::strum() {
    int chordScore = 0;
    qint64 currentTime = mediaPlayer->position();
    Chord* currentChord = &(currentDifficulty->at(currentScoreChord));
    std::array<bool, 5> currentChordBools = currentChord->getNotes();

    QString tmp = "At " + QString::number(currentTime) + ", got {";
    for (int i = 0; i < 5; i++) {
        tmp += (fretStates[i]) ? 'X' : ' ';
    }
    tmp += "} for chord " + QString::number(currentScoreChord) + " {";
    for (int i = 0; i < 5; i++) {
        tmp += (currentChordBools[i]) ? 'X' : ' ';
    }
    tmp += "} which ends at " + QString::number(currentChord->getStart() + TOLERANCE_DRAGGING);
    // Check if the strum is played on time:
    int diffWithNoteDrag = currentTime - currentChord->getStart();
    int diffWithNoteRush = currentChord->getStart() - currentTime;
    //qDebug() << "DiffRush = " + QString::number(diffWithNoteRush) + " --- DiffDrag = " + QString::number(diffWithNoteDrag);
    if ((diffWithNoteDrag < TOLERANCE_DRAGGING && diffWithNoteDrag > 0) || (diffWithNoteRush < TOLERANCE_RUSHING && diffWithNoteRush > 0)) {
        bool noteCorrectlyPlayed = true;
        for (int i = 0; i < 5; i++) {
            if (fretStates[i] != currentChordBools[i]) {
                //chordScore = SCORE_WRONG_NOTE;
                noteCorrectlyPlayed = false;
            }
        }
        if (noteCorrectlyPlayed)
        {
            chordScore = SCORE_GOOD_NOTE * currentChord->getNbNotes() * currentMultiplier;
            if (currentChord->getDuration() != 0)
            {
                qDebug() << QString::number(currentChord->getDuration());
                QTimer* longReleaseClock = new QTimer();
                longReleaseClock->setInterval(1);
                int test = currentScoreChord;
                connect(longReleaseClock, &QTimer::timeout, this, [=]() {longCheck(longReleaseClock, test);
                    });
                longReleaseClock->start();
                longNote = true;
            }
            correctlyPlayedNotes++;
            currentStreak++;
            if (currentStreak > maxStreak)
            {
                maxStreak = currentStreak;
            }
            if ((currentStreak % STREAK_TO_INCREASE_BARGRAPH == 0) && (bargraphState < 10) && (!activePowerup))
            {
                bargraphState++;
            }
            if (bargraphState == 10) {
                streakReady = true;
            }
            currentScoreChord++;
        }
        else
        {
            currentStreak = 0;
            chordScore = SCORE_SURPLUS_NOTE;
        }
        // std::array<bool,5> currentChordBools = currentChord->getNotes();
        // QString tmp = "Chord:" QString::number(currentScoreChord)
        // qDebug()

    }
    else
    {
        currentStreak = 0;
        chordScore = SCORE_SURPLUS_NOTE;
    }
    switch (chordScore) {
        /*case SCORE_LATE_NOTE:
          tmp += "-> LATE NOTE";
          break;
        case SCORE_WRONG_NOTE:
          tmp += "-> WRONG NOTE";
          break;*/
    case SCORE_GOOD_NOTE:
        tmp += "-> GOOD NOTE";
        break;
    case SCORE_SURPLUS_NOTE:
        tmp += "-> SURPLUS_NOTE";
        break;
    default:
        break;
    };
    qDebug() << tmp;
    highscore += chordScore;
    scene->getRightBar()->setScore(highscore);
    scene->getRightBar()->setStreak(currentStreak);
    scene->getRightBar()->setFilledRect(bargraphState);
    if (bargraphState == 10 && !activePowerup)
    {
        scene->getRightBar()->recolorFullBargraph();
    }
}

void Song::longCheck(QTimer* clock, uint chordIndex)
{
    bool needToResetClock = false;
    if (longNote)
    {
        qint64 currentTime = mediaPlayer->position();
        Chord* currentChord = &(currentDifficulty->at(chordIndex));
        std::array<bool, 5> currentChordBools = currentChord->getNotes();

        if (currentChord->getEnd() >= currentTime)
        {
            if (currentChord->getStart() <= currentTime)
            {
                bool noteCorrectlyPlayed = true;
                for (int i = 0; i < 5; i++) {
                    if (fretStates[i] != currentChordBools[i]) {
                        noteCorrectlyPlayed = false;
                    }
                }
                if (noteCorrectlyPlayed)
                {
                    highscore += 1 * currentChord->getNbNotes();
                    scene->getRightBar()->setScore(highscore);
                }
                else {
                    needToResetClock = true;
                }
            }
        }
        else {
            needToResetClock = true;
        }
        if (needToResetClock)
        {
            longNote = false;
            delete clock;
            clock = nullptr;
        }
    }
}

QString Song::getTitle() {
    qDebug() << title + " for path " + chartfile;
    return title;
}

QString Song::getArtist() {
    return artist;
}

QString Song::getAlbum() {
    return album;
}

QString Song::getYear() {
    return year;
}

QString Song::getCharter() {
    return charter;
}

QString Song::getScore() {
    return QString::number(highscore);
}

QString Song::getMaxStreak() {
    return QString::number(maxStreak);
}

int Song::getSongSize() {
    return currentDifficulty->size();
}

int Song::getCorrectlyPlayedNotes() {
    return correctlyPlayedNotes;
}

void Song::handleMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        qDebug() << "Reached end of song";
        QGraphicsView* view = scene->getView();
        view->setScene(new EndMenu(view, this));
        delete scene;
    }
}
