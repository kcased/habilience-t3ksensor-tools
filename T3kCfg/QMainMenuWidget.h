#ifndef QMAINMENUWIDGET_H
#define QMAINMENUWIDGET_H

#include <QWidget>
#include "QTPDPEventMultiCaster.h"
#include "QLangManager.h"
#include "QRequestHIDManager.h"

#include <QMap>

namespace Ui {
    class QMainMenuWidget;
}

class QMainMenuWidget : public QWidget, public QTPDPEventMultiCaster::ITPDPEventListener, public QLangManager::LangChangeNotify
{
    Q_OBJECT

public:
    explicit QMainMenuWidget(T3kHandle*& pHandle, QWidget *parent = 0);
    ~QMainMenuWidget();

    void RequestInformation();

protected:
    void UpdateInformation();

    virtual void showEvent(QShowEvent *evt);
    virtual void hideEvent(QHideEvent *evt);
    virtual void closeEvent(QCloseEvent *evt);

    virtual void OnChangeLanguage();

    virtual void OnRSP(ResponsePart Part, short lTickTime, const char *sPartId, long lId, bool bFinal, const char *sCmd);
    virtual void OnRSE(ResponsePart Part, short lTickTime, const char *sPartId, long lId, bool bFinal, const char *sCmd);

private:
    Ui::QMainMenuWidget *ui;

    T3kHandle*&            m_pT3kHandle;

    QString                 m_strMMVersion;
    QString                 m_strMMSN;
    float                   m_fMMVersion;
    int                     m_bMMNoCam;

    typedef struct _CMVerInfo
    {
        QString             strCMVersion;
        QString             strCMSN;
        QString             strSetDate;
        float               fCMVersion;
        short               nModel;
        int                 bNoCam;
    } CMVerInfo;

    QMap<int, CMVerInfo>    m_mapCMVerInfo;

    QRequestHIDManager      m_RequestSensorData;

    bool                    m_bSoftkey;
    bool                    m_bDigitizerMode;

signals:
    void ShowErrorMsgBox(QString str);
    void ShowMenuEvent(int nMenu);

private slots:
    void on_BtnMainMouse_clicked();
    void on_BtnMainCali_clicked();
    void on_BtnMainSensor_clicked();
    void on_BtnMainGeneral_clicked();
    void on_BtnMainSoftkey_clicked();
};

#endif // QMAINMENUWIDGET_H
