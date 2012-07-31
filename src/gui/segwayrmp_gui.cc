#include "segwayrmp/segwayrmp.h"
#include "segwayrmp/gui/segwayrmp_gui.h"
#include "ui_segwayrmp_gui.h"
#include "segwayrmp/impl/rmp_ftd2xx.h"

#include <QtCore/QtConcurrentRun>
#include <QtGui/QErrorMessage>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    connected_(false),
    interface_type_(segwayrmp::usb),
    rmp_type_(segwayrmp::rmp200)
{
    ui->setupUi(this);
    this->usb_devices_.push_back("No devices detected.");
    this->updateUSBList();

    // Connection related signals and slots
    connect(ui->connect_button, SIGNAL(clicked()), this, SLOT(onConnectClicked()));

    connect(ui->connection_type, SIGNAL(currentIndexChanged(QString)), this, SLOT(connectionTypeChanged(QString)));

    connect(ui->rmp_type, SIGNAL(currentIndexChanged(QString)), this, SLOT(rmpTypeChanged(QString)));

    connect(ui->refresh_button, SIGNAL(clicked()), this, SLOT(updateUSBList()));
    connect(this, SIGNAL(USBUpdateComplete()), this, SLOT(USBListUpdated()));

    // Logging connections
    connect(this, SIGNAL(segwayLog(QString)), this, SLOT(handleSegwayLog(QString)));

    // Segway Status connection
    connect(this, SIGNAL(segwayStatus(QString)), this, SLOT(handleSegwayStatus(QString)));

    // Command button connections
    connect(ui->reset_integrators_button, SIGNAL(clicked()), this, SLOT(onResetIntegrators()));
    connect(ui->disable_motors_button, SIGNAL(clicked()), this, SLOT(onDisableMotors()));
    connect(ui->tractor_button, SIGNAL(clicked()), this, SLOT(onRequestTractor()));
    connect(ui->balance_button, SIGNAL(clicked()), this, SLOT(onRequestBalance()));
    connect(ui->power_down_button, SIGNAL(clicked()), this, SLOT(onRequestPowerDown()));
    connect(ui->balance_lockout_button, SIGNAL(clicked()), this, SLOT(onBalanceLockout()));
    connect(ui->balance_unlock_button, SIGNAL(clicked()), this, SLOT(onBalanceUnlock()));
    connect(ui->send_config_button, SIGNAL(clicked()), this, SLOT(onSendConfig()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onResetIntegrators() {
    if (this->connected_ and this->rmp_) {
        rmp_->resetAllIntegrators();
    }
}

void MainWindow::onDisableMotors() {
    if (this->connected_ and this->rmp_) {
        rmp_->shutdown();
    }
}

void MainWindow::onRequestTractor() {
    if (this->connected_ and this->rmp_) {
        rmp_->setOperationalMode(segwayrmp::tractor);
    }
}

void MainWindow::onRequestBalance() {
    if (this->connected_ and this->rmp_) {
        rmp_->setOperationalMode(segwayrmp::balanced);
    }
}

void MainWindow::onRequestPowerDown() {
    if (this->connected_ and this->rmp_) {
        rmp_->setOperationalMode(segwayrmp::power_down);
    }
}

void MainWindow::onBalanceLockout() {
    if (this->connected_ and this->rmp_) {
        rmp_->setBalanceModeLocking(true);
    }
}

void MainWindow::onBalanceUnlock() {
    if (this->connected_ and this->rmp_) {
        rmp_->setBalanceModeLocking(false);
    }
}

void MainWindow::onSendConfig() {
    if (this->connected_ and this->rmp_) {
        rmp_->setMaxVelocityScaleFactor(ui->max_vel_sb->value());
        rmp_->setMaxAccelerationScaleFactor(ui->max_accel_sb->value());
        rmp_->setMaxTurnScaleFactor(ui->max_turn_rate_sb->value());
        rmp_->setCurrentLimitScaleFactor(ui->current_limit_sb->value());
        QString gain_schedule = ui->gain_schedule_cb->currentText();
        if (gain_schedule == "Light") {
            rmp_->setControllerGainSchedule(segwayrmp::light);
        }
        if (gain_schedule == "Tall") {
            rmp_->setControllerGainSchedule(segwayrmp::tall);
        }
        if (gain_schedule == "Heavy") {
            rmp_->setControllerGainSchedule(segwayrmp::heavy);
        }
        ui->statusbar->showMessage("Configurations sent to the Segway RMP", 3000);
    }
}

void MainWindow::onSegwayLog(QString log_type, const std::string &msg) {
    emit segwayLog(QString("%1: %2").arg(log_type, QString::fromStdString(msg)));
}

void MainWindow::handleSegwayLog(QString log) {
    qDebug() << log;
    ui->statusbar->showMessage(log, 3000);
}

void MainWindow::onSegwayStatus(segwayrmp::SegwayStatus::Ptr ss) {
    QString qss = QString::fromStdString(ss->str());
    emit segwayStatus(QString("Time Stamp:\n%1").arg(qss));
}

void MainWindow::handleSegwayStatus(QString ss) {
    ui->status_label->setText(ss);
}

void MainWindow::onConnectClicked() {
    if (!this->connected_) {
        try {
            rmp_ = new segwayrmp::SegwayRMP(this->interface_type_, this->rmp_type_);
            if (this->interface_type_ == segwayrmp::usb) {
                rmp_->configureUSBByIndex(ui->connection_id->currentIndex());
            } else if (this->interface_type_ == segwayrmp::serial) {
                rmp_->configureSerial(ui->connection_id->currentText().toStdString());
            }
            rmp_->setLogMsgCallback("error", boost::bind(&MainWindow::onSegwayLog, this, "Error", _1));
            rmp_->setLogMsgCallback("info", boost::bind(&MainWindow::onSegwayLog, this, "Info", _1));
            rmp_->setLogMsgCallback("debug", boost::bind(&MainWindow::onSegwayLog, this, "Debug", _1));
            rmp_->setStatusCallback(boost::bind(&MainWindow::onSegwayStatus, this, _1));
            rmp_->connect();
            this->connected_ = true;
            ui->connect_button->setText("Disconnect");
            ui->connection_id->setEnabled(false);
            ui->connection_type->setEnabled(false);
            ui->rmp_type->setEnabled(false);
            ui->refresh_button->setEnabled(false);
        } catch (const std::exception &e) {
            QString error = QString::fromAscii(e.what());
            QErrorMessage error_msg;
            QString error_string = QString("Error connecting: %1").arg(error);
            qDebug() << "Error: " << error_string;
            error_msg.showMessage(error_string);
            error_msg.exec();
            delete rmp_;
            rmp_ = NULL;
            this->connected_ = false;
        }
    } else {
        delete rmp_;
        rmp_ = NULL;
        this->connected_ = false;
        ui->connect_button->setText("Connect");
        ui->connection_id->setEnabled(true);
        ui->connection_type->setEnabled(true);
        ui->rmp_type->setEnabled(true);
        ui->refresh_button->setEnabled(true);
    }
}

void MainWindow::connectionTypeChanged(QString mode) {
    if (mode == "USB") {
        ui->refresh_button->setEnabled(true);
        ui->connection_id->setEditable(false);
        this->updateUSBList();
        this->interface_type_ = segwayrmp::usb;
    } else if (mode == "Serial") {
        ui->refresh_button->setEnabled(false);
        ui->connection_id->setEditable(true);
        ui->connection_id->clear();
        ui->connection_id->addItem(QString("/dev/ttyUSB0"));
        this->interface_type_ = segwayrmp::serial;
    }
}

void MainWindow::rmpTypeChanged(QString type) {
    if (type == "rmp50") {
        this->rmp_type_ = segwayrmp::rmp50;
    }
    if (type == "rmp100") {
        this->rmp_type_ = segwayrmp::rmp100;
    }
    if (type == "rmp200") {
        this->rmp_type_ = segwayrmp::rmp200;
    }
    if (type == "rmp400") {
        this->rmp_type_ = segwayrmp::rmp400;
    }
}

void MainWindow::USBListUpdated() {
    ui->connection_id->clear();
    foreach (QString entry, this->usb_devices_) {
        ui->connection_id->addItem(entry);
    }
}

void MainWindow::updateUSBList() {
    QtConcurrent::run(this, &MainWindow::updateUSBList_);
}

void MainWindow::updateUSBList_() {
    std::vector<FT_DEVICE_LIST_INFO_NODE> devices = segwayrmp::enumerateUSBDevices();
    std::vector<FT_DEVICE_LIST_INFO_NODE>::iterator it;
    this->usb_devices_.clear();
    int i = 0;
    if (devices.size() == 0) {
        this->usb_devices_.push_back("No devices detected.");
    }
    for (it = devices.begin(); it != devices.end(); it++) {
        QString line = QString("%1 : %2 - %3").arg(QString::number(i), QString(it->Description), QString(it->SerialNumber));
        this->usb_devices_.push_back(line);
    }
    emit USBUpdateComplete();
}

