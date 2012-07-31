#ifndef SEGWAYRMP_GUI_H
#define SEGWAYRMP_GUI_H

#include <QtGui/QMainWindow>
#include <QtCore/QMutex>

#include <SDL/SDL.h>

#include <segwayrmp/segwayrmp.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public slots:
    void onConnectClicked();
    void connectionTypeChanged(QString);
    void rmpTypeChanged(QString);
    void USBListUpdated();
    void updateUSBList();

    void updateJoysticks();
    void onJoystickChanged(int);
    void commandPoll();

    void handleSegwayLog(QString);
    void handleSegwayStatus(QString);

    void onResetIntegrators();
    void onDisableMotors();
    void onRequestTractor();
    void onRequestBalance();
    void onRequestPowerDown();
    void onBalanceLockout();
    void onBalanceUnlock();
    void onSendConfig();

signals:
    void USBUpdateComplete();
    void segwayLog(QString);
    void segwayStatus(QString);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void onSegwayLog(QString, const std::string&);
    void onSegwayStatus(segwayrmp::SegwayStatus::Ptr);
    
private:
    Ui::MainWindow *ui;
    bool connected_;
    QVector<QString> usb_devices_;
    segwayrmp::SegwayRMP * rmp_;
    segwayrmp::InterfaceType interface_type_;
    segwayrmp::SegwayRMPType rmp_type_;
    SDL_Joystick * joystick_;
    QMutex joy_mutex_;
    bool running_;

    void updateUSBList_();

protected:
    void closeEvent(QCloseEvent*);
};

#endif // SEGWAYRMP_GUI_H
