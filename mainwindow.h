#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <wiringPi.h>
#include <softPwm.h>

#include <QtMqtt/QtMqtt>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttMessage>
#include <QtMqtt/QMqttSubscription>
#include <QTcpSocket>

#include <QSerialPort>

#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void timerAction();
	void animationAction();

	void buttonAction();
	void sliderAction();

	void connectAction();
	void subscriptionsAction();
	void mqttOnOffAction(QMqttMessage message);
	void mqttValueAction(QMqttMessage message);

private:
	Ui::MainWindow *ui;

	QTimer * timer;
	QTimer * animationTimer;

	bool switchState;
	bool lightState;

	double pwmValue;
	double actualPwmValue;

	QMqttClient * mqttClient;
	//QMqttSubscription * subscriptionOnOff;
	//QMqttSubscription * subscriptionValue;
};

#endif // MAINWINDOW_H
