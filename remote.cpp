#include "remote.h"

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <array>

#include "include/serial/SerialPort.hpp"
#include "include/json.hpp"

using json = nlohmann::json;

Remote::Remote(QApplication* app, int baud, std::string com) {
	this->app = app;
	// Set default values to inputs
	fretStates = { false,false,false,false,false };
	strumStates = { false,false };
	joyState = 7;
	// Initialise Arduino
	port = new SerialPort(com.c_str(), baud);
	// Setup constant polling of arduino serial (every 10ms)
	clock = new QTimer();
	clock->setInterval(10); // Rechecks every 10 ms
		connect(clock, &QTimer::timeout, this, &Remote::checkArduino);
	clock->start();
}

Remote::~Remote() {
	if (clock != NULL) delete clock;
	if (port != NULL) delete port;
}

void Remote::checkArduino() {
	// Default keyboard values of certain frets
	Qt::Key fretKeys[5] = {
		Qt::Key_A, Qt::Key_S, Qt::Key_D, Qt::Key_F, Qt::Key_G
	};
	Qt::Key joyKeys[2] = { Qt::Key_Z, Qt::Key_X };
	Qt::Key strumKey = Qt::Key_Return;
	// Don't try to check this info if the arduino isn't connected
	if (!port->isConnected()) return;
	// Read raw data from the arduino
	std::string strBuffer;
	char charBuffer[MSG_MAX_SIZE];
	int bufferSize = port->readSerialPort(charBuffer, MSG_MAX_SIZE);
	qDebug() << charBuffer;
	strBuffer.assign(charBuffer, bufferSize);
	// initialize temporary info
	std::array<bool, 5> newFretStates;
	std::array<bool, 2> newStrumStates;
	int newJoyState;
	int newShakeState;
	int newMuonState;
	// Parse the json to assing the info to the previous values
	
	if (strBuffer.size() > 0)  {
	    json json_rcvd = json::parse(strBuffer);
    	if (json_rcvd = json::value_t::null) {
    		qDebug() << "No data parsed";
    		return;
    	}
		// Parse the frets
		for (int i = 0;i < 5;i++) {
			newFretStates[i] = json_rcvd[i];
		}
		// Parse the strumming
		for (int i = 0;i < 2;i++) {
			newStrumStates[i] = json_rcvd[i + 5];
		}
	}
	// Compare the received data to the previous data (contained in the object)
	for (int i = 0;i < 5;i++) {
		if (newFretStates[i] != fretStates[i]) {
			// THERE WAS A CHANGE, LET's EMIT A KEY THAT CORRESPONDS:
			switch (newFretStates[i]) {
				case PRESSED:
					break;
				case RELEASED:
					break;
				default:
					break;
			}
		}
	}
	
	QString tmp = "{";
	for (int i = 0;i < 5;i++) {
		tmp += (newFretStates[i]) ? 'X' : ' ';
	};
	tmp += "}";
	QString buff = QString::fromStdString(strBuffer);
	qDebug() << buff;
}

