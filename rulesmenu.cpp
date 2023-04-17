#include "rulesmenu.h"
#include "mainmenu.h"
#include "ui.h"
#include "common.h"

#include <QColor>
#include <QFont>
#include <QKeyEvent>
#include <QGraphicsTextItem>
#include <QMediaPlayer>

RulesMenu::RulesMenu(QGraphicsView* view, QObject* parent) {
  this->view = view;
  this->setSceneRect(view->rect());
  // Create click sound
  btnSound = new MediaPlayer();
  btnSound->setMedia(QUrl::fromLocalFile(clickSoundPath));
  // Set background
  setBgGradient(this);

  const char* rulesText = R"(
Le but du jeu consiste à frapper les bonnes notes telles qu'elles
arrivent à l'écran.

Une note est considérée comme frappée si la frette qui lui
correspond est appuyée au moment où la barre de strumming
est frappée.

Si une note est longue, des points supplémentaires seront accordés
si la frette est maintenue jusqu'à la fin de la note.

Il est possible que plusieurs notes aient à être appuyées simultanément.
Il est donc important de s'assurer que seulement ces frettes là
et qu'aucune autre ne soient appuyées

La règle la plus importante de toutes, c'est d'avoir du plaisir
ET DE GARDER LE ROCK VIVANT!
                         )";

  title = new QGraphicsTextItem();
  title->setPlainText("Règles du jeu:");
  title->setDefaultTextColor(TEXT_COLOR_MAIN);
  title->setFont(QFont("Arial",64));

  paragraph = new QGraphicsTextItem();
  paragraph->setPlainText(rulesText);
  paragraph->setDefaultTextColor(TEXT_COLOR_SECONDARY);
  paragraph->setFont(QFont("Arial",18));

  backButton = new QGraphicsTextItem();
  backButton->setPlainText("Retour en arrière");
  backButton->setDefaultTextColor(TEXT_COLOR_MENU);
  backButton->setFont(QFont("Arial",24));

  title->setPos(TEXT_SIDE_PADDING,TEXT_TOP_PADDING);
  paragraph->setPos(TEXT_SIDE_PADDING,
    title->pos().y()+title->boundingRect().height()+TEXT_MARGIN_Y );
  backButton->setPos(TEXT_SIDE_PADDING,
    paragraph->pos().y()+paragraph->boundingRect().height()+TEXT_MARGIN_Y );

  this->addItem(title);
  this->addItem(paragraph);
  this->addItem(backButton);

}

RulesMenu::~RulesMenu(){
  if (btnSound!=NULL) delete btnSound;
  if (title!=NULL) delete title;
  if (paragraph!=NULL) delete paragraph;
  if (backButton!=NULL) delete backButton;
}

void RulesMenu::select(){
  view->setScene(new MainMenu(view));
  delete this;
}

void RulesMenu::keyPressEvent(QKeyEvent* event){
  switch (event->key()){
    case Qt::Key_A:
      select();
      break;
    default:
      break;
    }
}

void RulesMenu::keyReleaseEvent(QKeyEvent* event){}
