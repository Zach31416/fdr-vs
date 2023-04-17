#include "infomenu.h"
#include "mainmenu.h"
#include "ui.h"
#include "common.h"

#include <QColor>
#include <QFont>
#include <QKeyEvent>
#include <QGraphicsTextItem>

InfoMenu::InfoMenu(QGraphicsView* view, QObject* parent) {
  this->view = view;
  this->setSceneRect(view->rect());
  // Set background
  setBgGradient(this);

  const char* rulesText = R"(
Le jeu "Flaming Devillish Relish" est un hommage au jeu créé par
RedOctane "Guitar Hero". Sa création fut un exercice technique
ayant aidé à l'approfondissmenent des connaissances informatiques,
mathématiques et électriques des 6 étudiants l'ayant réalisé dans le
cadre du cours de projet de la deuxième session du baccalauréat à
l'Université de Sherbrooke. Celui-ci a été conçu avec la librairie
visuelle QT. La totalité du code lié à ce projet demeure sous la
license LGPL tel que QT le demande pour pouvoir en faire une utilisation
gratuite. Du fond de notre coeur, nous, les créateurs du jeu, vous remercions
d'essayer notre jeu.

- Benjamin C.
- Brenda T.
- Gabriel N.
- Guillaume M.
- Simon G.
- Zachary P.
                         )";

  title = new QGraphicsTextItem();
  title->setPlainText("Informations sur le jeu:");
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

InfoMenu::~InfoMenu(){
  if (title!=NULL) delete title;
  if (paragraph!=NULL) delete paragraph;
  if (backButton!=NULL) delete backButton;
}

void InfoMenu::select(){
  view->setScene(new MainMenu(view));
  delete this;
}

void InfoMenu::keyPressEvent(QKeyEvent* event){
  switch (event->key()){
    case Qt::Key_A:
      select();
      break;
    default:
      break;
    }
}

void InfoMenu::keyReleaseEvent(QKeyEvent* event){}
