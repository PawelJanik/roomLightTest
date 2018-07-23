#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	if(wiringPiSetup () == -1)
		exit(1);
	pinMode(29, INPUT);
	pinMode(1, OUTPUT);

	softPwmCreate (1, 0, 100) ;

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()),this , SLOT(timerAction()));
	timer->start(10);

	animationTimer = new QTimer(this);
	connect(animationTimer, SIGNAL(timeout()),this , SLOT(animationAction()));
	animationTimer->start(30);

	lightState = false;

	if (digitalRead(29))
	{
			switchState = true;
	}
	else
	{
			switchState = false;
	}

	connect(ui->pushButton, SIGNAL(clicked(bool)),this,SLOT(buttonAction()));
	connect(ui->Slider, SIGNAL(valueChanged(int)),this,SLOT(sliderAction()));

	mqttClient = new QMqttClient(this);
	mqttClient->setHostname("192.168.1.100");
	mqttClient->setPort(1883);

	connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(connectAction()));
	connect(ui->subscriptionsButton, SIGNAL(clicked(bool)), this, SLOT(subscriptionsAction()));

	QTimer::singleShot(1,this,SLOT(connectAction()));

	QTimer::singleShot(1000,this,SLOT(subscriptionsAction()));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::connectAction()
{
	qDebug() << "Connecting...";
	mqttClient->connectToHost();
	while(mqttClient->state() == QMqttClient::Connected){}
	qDebug() << "Connected";
}

void MainWindow::subscriptionsAction()
{
	qDebug() << "Subscripting...";

	QMqttTopicFilter topicOnOff("lightonoff");
	auto subscriptionOnOff = mqttClient->subscribe(topicOnOff, 0);
	connect(subscriptionOnOff, SIGNAL(messageReceived(QMqttMessage)), this, SLOT(mqttOnOffAction(QMqttMessage)));

	while(subscriptionOnOff->state() == QMqttSubscription::Subscribed){}

	qDebug() << "subscription: lightonoff	ok";

	QMqttTopicFilter topicValue("lightvalue");
	auto subscriptionValue = mqttClient->subscribe(topicValue, 0);
	connect(subscriptionValue, SIGNAL(messageReceived(QMqttMessage)), this, SLOT(mqttValueAction(QMqttMessage)));

	while(subscriptionValue->state() == QMqttSubscription::Subscribed){}

	qDebug() << "subscription: lightvalue	ok";
}

void MainWindow::timerAction()
{
	bool change = false;

	if (digitalRead(29))
	{
		if(switchState == false)
		{
			switchState = true;
			change = true;
		}
	}
	else
	{
		if(switchState == true)
		{
			switchState = false;
			change = true;
		}
	}

	if(change == true)
	{
		if(lightState)
		{
			ui->pushButton->setText("On");
			lightState = false;
			mqttClient->publish(QMqttTopicName("lightonoff"),QByteArray("0"),1,true);
		}
		else
		{
			ui->pushButton->setText("Off");
			lightState = true;
			mqttClient->publish(QMqttTopicName("lightonoff"),QByteArray("1"),1,true);
		}

		qDebug() << "change";
	}

	if(lightState)
	{
		pwmValue = ui->Slider->value()/10.0;
	}
	else
	{
		pwmValue = 0;
	}

	double s = pow(2, actualPwmValue)/10;
	softPwmWrite(1, s);

	//qDebug() << "pwmValue: " << pwmValue << " s: " << s;
}

void MainWindow::animationAction()
{

	if((pwmValue > actualPwmValue) && (actualPwmValue < 10) && lightState)
		actualPwmValue = actualPwmValue + 0.1;
	else if((pwmValue < actualPwmValue) && (actualPwmValue > 3.3))
		actualPwmValue = actualPwmValue - 0.1;

}

void MainWindow::buttonAction()
{
	if(lightState)
	{
		ui->pushButton->setText("On");
		mqttClient->publish(QMqttTopicName("lightonoff"),QByteArray("0"),1,true);
		lightState = false;

	}
	else
	{
		ui->pushButton->setText("Off");
		mqttClient->publish(QMqttTopicName("lightonoff"),QByteArray("1"),1,true);
		lightState = true;
	}

	qDebug() << "change";
}

void MainWindow::sliderAction()
{
	QByteArray value = QByteArray::number(ui->Slider->value());
	mqttClient->publish(QMqttTopicName("lightvalue"), value,0);
}

void MainWindow::mqttOnOffAction(QMqttMessage message)
{
	if(message.payload() == "0")
		lightState = false;
	else
		lightState = true;

	qDebug() << "Mqtt message: " << QString(message.payload());
}

void MainWindow::mqttValueAction(QMqttMessage message)
{
	ui->Slider->setValue(message.payload().toDouble());

	ui->pushButton->setText("Off");
	mqttClient->publish(QMqttTopicName("lightonoff"),QByteArray("1"),0,true);
	lightState = true;

	qDebug() << "Mqtt message: " << QString(message.payload());
}
